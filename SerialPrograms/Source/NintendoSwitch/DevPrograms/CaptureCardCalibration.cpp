/*  Capture Card Calibration
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  See the header for the overview. This file implements:
 *    - Sampling the color-picker patches out of a capture-card frame (auto-detecting the
 *      console layout and which page of the grid is shown).
 *    - Matching the sampled colors against every profile in
 *      CaptureCardCalibration/color_profile_to_transform.json.
 *    - Fitting the raw 3x4 color transform when no profile matches.
 *
 *  The color math mirrors CommandLineTests/CaptureCardCalibration/measure_capture_card_colors.py:
 *  a capture card is modeled as `captured = clip(M @ reference + b, 0, 255)` on RGB in 0-255,
 *  and the error is a clip-aware RMS (clipped captured values only count as wrong when the
 *  prediction is on the wrong side of the clip).
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>
#include "Common/Cpp/CancellableScope.h"
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/TestRunners/TestImageDistortionManager.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CaptureCardCalibration.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

namespace{


//  Captured channel values at or beyond these are treated as clipped by the capture pipeline.
constexpr double CLIP_LOW = 0.5;
constexpr double CLIP_HIGH = 254.5;

//  Side of the sampled square as a fraction of the patch size. Only the center of each patch is
//  averaged so that the selection cursor and the "current background" outline don't bleed in
//  (capture cards blur and chroma-subsample).
constexpr double SAMPLE_FRACTION = 0.4;

//  A sampled square whose per-channel standard deviation is above this is not flat, which usually
//  means the frame isn't on the color picker or the layout doesn't match the image.
constexpr double FLATNESS_WARNING_STDDEV = 4.0;

using Color3 = std::array<double, 3>;

//  The 60 reference colors of the Switch user-icon "Edit Background" color picker, in the order
//  the grid is read row-major: Red0..Red5, Orange0..5, Yellow0..5, YellowGreen0..5, Green0..5,
//  then Blue0..5, Purple0..5, Pink0..5, Gray0..5, BlueGray0..5. The "Up" page shows the first 30
//  (the top 5 rows) and the "Down" page the last 30 (the bottom 5 rows).
//
//  These are the ground-trund colors, averaged from the undistorted "Reference" calibration pairs
//  in color_profile_labels.json (the CFW-BrightBG / CFW-DarkBG Switch 1 dumps). They are the same
//  on every console, so a single table serves both layouts.
const Color3 REFERENCE_COLORS[60] = {
    {255.0000, 205.0000, 204.0000},  // Red0
    {255.0000, 153.0000, 154.0000},  // Red1
    {255.0000, 105.0000, 104.0000},  // Red2
    {254.0000,  0.0000,   0.0000},  // Red3
    {200.0000,  10.0000,  10.0000},  // Red4
    {111.0000,  15.0000,  16.0000},  // Red5
    {255.0000, 227.0000, 205.0000},  // Orange0
    {255.0000, 196.0000, 154.0000},  // Orange1
    {255.0000, 176.0000, 101.0000},  // Orange2
    {255.0000, 133.0000,   0.0000},  // Orange3
    {194.0000, 127.0000,  57.0000},  // Orange4
    {109.0000,  71.0000,  34.0000},  // Orange5
    {255.0000, 255.0000, 154.0000},  // Yellow0
    {255.0000, 255.0000,   0.0000},  // Yellow1
    {255.0000, 231.0000,   0.0000},  // Yellow2
    {255.0000, 182.0000,   0.0000},  // Yellow3
    {191.0000, 167.0000,  69.0000},  // Yellow4
    {125.0000, 109.0000,  14.0000},  // Yellow5
    {240.0000, 255.0000, 152.0000},  // YellowGreen0
    {230.0000, 255.0000,  49.0000},  // YellowGreen1
    {212.0000, 241.0000,  37.0000},  // YellowGreen2
    {198.0000, 215.0000,   0.0000},  // YellowGreen3
    {162.0000, 176.0000,  37.0000},  // YellowGreen4
    {103.0000, 115.0000,  15.0000},  // YellowGreen5
    {212.0000, 250.0000, 199.0000},  // Green0
    {180.0000, 244.0000, 157.0000},  // Green1
    {135.0000, 231.0000, 108.0000},  // Green2
    { 50.0000, 216.0000,  12.0000},  // Green3
    { 80.0000, 163.0000,  29.0000},  // Green4
    { 36.0000, 117.0000,  24.0000},  // Green5
    {174.0000, 214.0000, 249.0000},  // Blue0
    {128.0000, 189.0000, 254.0000},  // Blue1
    { 71.0000, 155.0000, 254.0000},  // Blue2
    { 48.0000,  79.0000, 255.0000},  // Blue3
    {  0.0000,  81.0000, 189.0000},  // Blue4
    { 28.0000,  61.0000, 104.0000},  // Blue5
    {228.0000, 204.0000, 254.0000},  // Purple0
    {211.0000, 169.0000, 255.0000},  // Purple1
    {181.0000, 107.0000, 254.0000},  // Purple2
    {140.0000,  24.0000, 255.0000},  // Purple3
    {113.0000,  29.0000, 189.0000},  // Purple4
    { 69.0000,  18.0000, 119.0000},  // Purple5
    {255.0000, 204.0000, 250.0000},  // Pink0
    {255.0000, 167.0000, 228.0000},  // Pink1
    {255.0000, 115.0000, 212.0000},  // Pink2
    {255.0000,  25.0000, 222.0000},  // Pink3
    {191.0000,   9.0000, 189.0000},  // Pink4
    {105.0000,  20.0000, 121.0000},  // Pink5
    {245.0000, 245.0000, 245.0000},  // Gray0
    {217.0000, 217.0000, 217.0000},  // Gray1
    {179.0000, 179.0000, 179.0000},  // Gray2
    {128.0000, 128.0000, 128.0000},  // Gray3
    { 71.0000,  75.0000,  86.0000},  // Gray4
    { 40.0000,  40.0000,  40.0000},  // Gray5
    {236.0000, 241.0000, 247.0000},  // BlueGray0
    {195.0000, 202.0000, 218.0000},  // BlueGray1
    {153.0000, 160.0000, 179.0000},  // BlueGray2
    {101.0000, 108.0000, 127.0000},  // BlueGray3
    { 60.0000,  68.0000,  89.0000},  // BlueGray4
    { 19.0000,  28.0000,  69.0000},  // BlueGray5
};

//  Pixel position of the visible part of each color patch, in 1920x1080 coordinates.
//  `up_rows` / `down_rows` are the (top, bottom) pixel rows of each of the 5 visible rows on the
//  Up / Down page. Some rows are clipped by the scroll area, so their visible height is smaller
//  than `patch_size`. Frames of other resolutions are scaled from these.
//  Measured the same way as the calibration script (see LAYOUTS in measure_capture_card_colors.py).
struct PatchLayout{
    double left;          // x of the left edge of column 0
    double pitch_x;       // distance between the left edges of neighboring columns
    double patch_size;    // width of a patch
    double up_rows[5][2];  // (top, bottom) of each visible row on the Up page
    double down_rows[5][2]; // (top, bottom) of each visible row on the Down page
};

//  Switch 1 uses the same patch rows on both pages.
const PatchLayout LAYOUT_SWITCH1 = {
    133.5, 153.0, 138.0,
    {
        {177 + 153 * 0, 177 + 153 * 0 + 138},
        {177 + 153 * 1, 177 + 153 * 1 + 138},
        {177 + 153 * 2, 177 + 153 * 2 + 138},
        {177 + 153 * 3, 177 + 153 * 3 + 138},
        {177 + 153 * 4, 177 + 153 * 4 + 138},
    },
    {
        {177 + 153 * 0, 177 + 153 * 0 + 138},
        {177 + 153 * 1, 177 + 153 * 1 + 138},
        {177 + 153 * 2, 177 + 153 * 2 + 138},
        {177 + 153 * 3, 177 + 153 * 3 + 138},
        {177 + 153 * 4, 177 + 153 * 4 + 138},
    },
};

//  Switch 2 scrolls differently: the last Up row (Green) is clipped at the bottom and the first
//  Down row (Blue) is clipped at the top of the scroll area.
const PatchLayout LAYOUT_SWITCH2 = {
    844.0, 160.0, 144.0,
    {
        {200, 344},
        {360, 504},
        {520, 664},
        {680, 824},
        {840, 968},
    },
    {
        {127, 264},
        {280, 424},
        {440, 584},
        {600, 744},
        {760, 904},
    },
};

//  The (console layout, page) combinations to try when auto-detecting the layout.
struct Combo{
    const char* console;
    bool up;
    const PatchLayout* layout;
};
const Combo COMBOS[4] = {
    {"Switch 1", true,  &LAYOUT_SWITCH1},
    {"Switch 1", false, &LAYOUT_SWITCH1},
    {"Switch 2", true,  &LAYOUT_SWITCH2},
    {"Switch 2", false, &LAYOUT_SWITCH2},
};



double clip01_255(double v){
    return v < 0.0 ? 0.0 : (v > 255.0 ? 255.0 : v);
}

//  Apply a 3x4 color transform to a single color, clipping to 0-255.
Color3 apply_transform(const ColorTransform3x4& m, const Color3& c){
    Color3 r;
    for (size_t ch = 0; ch < 3; ch++){
        r[ch] = clip01_255(m[ch][0] * c[0] + m[ch][1] * c[1] + m[ch][2] * c[2] + m[ch][3]);
    }
    return r;
}

//  Clip-aware RMS color error between `transform`'s prediction of `reference` and `captured`,
//  in 0-255 levels. Captured values clipped at 0 / 255 only count as an error when the
//  prediction is on the wrong side of the clip.
double clip_aware_rms(const ColorTransform3x4& m, const Color3* reference, const Color3* captured, size_t n){
    double sum = 0.0;
    for (size_t i = 0; i < n; i++){
        const Color3 pred = apply_transform(m, reference[i]);
        for (size_t ch = 0; ch < 3; ch++){
            double r = pred[ch] - captured[i][ch];
            if (captured[i][ch] >= CLIP_HIGH){
                r = std::min(r, 0.0);
            }else if (captured[i][ch] <= CLIP_LOW){
                r = std::max(r, 0.0);
            }
            sum += r * r;
        }
    }
    return std::sqrt(sum / (double)(n * 3));
}

//  Average the color of each visible patch. Fills `out` with 30 colors (row-major, matching the
//  order of REFERENCE_COLORS for the requested page) and sets `max_stddev` to the largest
//  per-patch per-channel standard deviation.
void sample_patches(const ImageViewRGB32& image, const PatchLayout& layout, bool up, std::vector<Color3>& out, double& max_stddev){
    out.clear();
    out.reserve(30);
    const int W = (int)image.width();
    const int H = (int)image.height();
    const double sx = (double)W / 1920.0;
    const double sy = (double)H / 1080.0;
    const double (*rows)[2] = up ? layout.up_rows : layout.down_rows;

    max_stddev = 0.0;
    for (int row = 0; row < 5; row++){
        const double top = rows[row][0];
        const double bottom = rows[row][1];
        const double visible_h = bottom - top;
        for (int col = 0; col < 6; col++){
            const double cx = layout.left + col * layout.pitch_x + layout.patch_size / 2.0;
            const double cy = (top + bottom) / 2.0;
            const double half = SAMPLE_FRACTION * std::min(layout.patch_size, visible_h) / 2.0;

            int x0 = (int)std::llround((cx - half) * sx);
            int y0 = (int)std::llround((cy - half) * sy);
            int x1 = (int)std::llround((cx + half) * sx);
            int y1 = (int)std::llround((cy + half) * sy);
            x0 = std::max(0, std::min(x0, W - 1));
            y0 = std::max(0, std::min(y0, H - 1));
            x1 = std::max(0, std::min(x1, W));
            y1 = std::max(0, std::min(y1, H));
            if (x1 <= x0 || y1 <= y0){
                out.push_back({0.0, 0.0, 0.0});
                continue;
            }

            double sum[3] = {0, 0, 0};
            double sumsq[3] = {0, 0, 0};
            long cnt = 0;
            for (int yy = y0; yy < y1; yy++){
                for (int xx = x0; xx < x1; xx++){
                    const uint32_t p = image.pixel(xx, yy);
                    const double r = (p >> 16) & 0xff;
                    const double g = (p >> 8) & 0xff;
                    const double b = (p >> 0) & 0xff;
                    sum[0] += r; sum[1] += g; sum[2] += b;
                    sumsq[0] += r * r; sumsq[1] += g * g; sumsq[2] += b * b;
                    cnt++;
                }
            }
            if (cnt == 0){
                out.push_back({0.0, 0.0, 0.0});
                continue;
            }
            double mean[3], stddev_max = 0.0;
            for (int c = 0; c < 3; c++){
                mean[c] = sum[c] / (double)cnt;
                const double variance = std::max(0.0, sumsq[c] / (double)cnt - mean[c] * mean[c]);
                stddev_max = std::max(stddev_max, std::sqrt(variance));
            }
            max_stddev = std::max(max_stddev, stddev_max);
            out.push_back({mean[0], mean[1], mean[2]});
        }
    }
}


//  Solve a 4x4 linear system A x = b with Gaussian elimination and partial pivoting.
//  On success, `b` is overwritten with the solution and true is returned.
void solve4(double A[4][4], double b[4]){
    for (int col = 0; col < 4; col++){
        int pivot = col;
        for (int r = col + 1; r < 4; r++){
            if (std::abs(A[r][col]) > std::abs(A[pivot][col])){
                pivot = r;
            }
        }
        if (std::abs(A[pivot][col]) < 1e-12){
            return;
        }
        if (pivot != col){
            for (int c = col; c < 4; c++){
                std::swap(A[pivot][c], A[col][c]);
            }
            std::swap(b[pivot], b[col]);
        }
        for (int r = col + 1; r < 4; r++){
            const double factor = A[r][col] / A[col][col];
            if (factor == 0.0){
                continue;
            }
            for (int c = col; c < 4; c++){
                A[r][c] -= factor * A[col][c];
            }
            b[r] -= factor * b[col];
        }
    }
    for (int r = 3; r >= 0; r--){
        double s = b[r];
        for (int c = r + 1; c < 4; c++){
            s -= A[r][c] * b[c];
        }
        b[r] = s / A[r][r];
    }
}

//  Least squares on a (n x 4) design matrix `D` (row-major): find c minimizing ||D c - y||^2.
//  Solves the 4x4 normal equations. `y` and `c` have 4 / n entries respectively.
void least_squares_4(const double* D, int n, const double* y, double c[4]){
    double A[4][4] = {};
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            double s = 0.0;
            for (int r = 0; r < n; r++){
                s += D[r * 4 + i] * D[r * 4 + j];
            }
            A[i][j] = s;
        }
    }
    double b[4] = {};
    for (int j = 0; j < 4; j++){
        double s = 0.0;
        for (int r = 0; r < n; r++){
            s += D[r * 4 + j] * y[r];
        }
        b[j] = s;
    }
    solve4(A, b);
    for (int j = 0; j < 4; j++){
        c[j] = b[j];
    }
}

//  Fit `captured = clip(M @ reference + b, 0, 255)` and return the (3, 4) matrix [M | b].
//  Port of `fit_affine()` in measure_capture_card_colors.py: the clip makes the problem piecewise
//  linear, so it is solved with Gauss-Newton steps, re-solving least squares on the samples whose
//  current prediction is inside (0, 255).
ColorTransform3x4 fit_raw_transform(const Color3* reference, const Color3* captured, size_t n){
    double design[30][4];
    for (size_t i = 0; i < n; i++){
        design[i][0] = reference[i][0];
        design[i][1] = reference[i][1];
        design[i][2] = reference[i][2];
        design[i][3] = 1.0;
    }

    ColorTransform3x4 m = {};
    for (int ch = 0; ch < 3; ch++){
        double y[30];
        for (size_t i = 0; i < n; i++){
            y[i] = captured[i][ch];
        }

        auto predict = [&](const double coef[4], double pred[30]){
            for (size_t i = 0; i < n; i++){
                pred[i] = clip01_255(design[i][0] * coef[0] + design[i][1] * coef[1] + design[i][2] * coef[2] + design[i][3] * coef[3]);
            }
        };
        auto loss = [&](const double coef[4]) -> double{
            double pred[30];
            predict(coef, pred);
            double s = 0.0;
            for (size_t i = 0; i < n; i++){
                double r = pred[i] - y[i];
                if (y[i] >= CLIP_HIGH){
                    r = std::min(r, 0.0);
                }else if (y[i] <= CLIP_LOW){
                    r = std::max(r, 0.0);
                }
                s += r * r;
            }
            return s;
        };
        auto solve_subset = [&](const int* idx, int count, double coef[4]){
            double Dsub[30][4];
            double ysub[30];
            for (int k = 0; k < count; k++){
                for (int j = 0; j < 4; j++){
                    Dsub[k][j] = design[idx[k]][j];
                }
                ysub[k] = y[idx[k]];
            }
            least_squares_4(&Dsub[0][0], count, ysub, coef);
        };

        double coef[4];
        {
            //  Start from a plain least-squares fit on the samples whose captured value is not clipped.
            int idx[30];
            int ni = 0;
            for (size_t i = 0; i < n; i++){
                if (y[i] > CLIP_LOW && y[i] < CLIP_HIGH){
                    idx[ni++] = (int)i;
                }
            }
            if (ni < 4){
                ni = (int)std::min<size_t>(n, 30);
                for (int k = 0; k < ni; k++){
                    idx[k] = k;
                }
            }
            solve_subset(idx, ni, coef);
        }

        double current = loss(coef);
        for (int iter = 0; iter < 50; iter++){
            double pred[30];
            predict(coef, pred);
            int idx[30];
            int nlin = 0;
            for (size_t i = 0; i < n; i++){
                if (pred[i] > 0.0 && pred[i] < 255.0){
                    idx[nlin++] = (int)i;
                }
            }
            if (nlin < 4){
                break;
            }
            double candidate[4];
            solve_subset(idx, nlin, candidate);

            //  Step towards the candidate, halving until the loss decreases.
            double step = 1.0;
            double trial[4];
            auto apply_step = [&](double s){
                for (int j = 0; j < 4; j++){
                    trial[j] = coef[j] + s * (candidate[j] - coef[j]);
                }
            };
            while (step > 1e-3){
                apply_step(step);
                if (loss(trial) < current){
                    break;
                }
                step /= 2.0;
            }
            if (step <= 1e-3){
                break;
            }
            apply_step(step);
            for (int j = 0; j < 4; j++){
                coef[j] = trial[j];
            }
            current = loss(coef);
        }
        for (int j = 0; j < 4; j++){
            m[ch][j] = coef[j];
        }
    }
    return m;
}

std::string fnum(double v, int precision = 2){
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.*f", precision, v);
    return std::string(buf);
}

//  Format a 3x4 transform the way color_profile_to_transform.json writes it.
std::string format_transform(const ColorTransform3x4& m){
    std::string s = "{";
    for (int r = 0; r < 3; r++){
        s += "\n      [";
        for (int c = 0; c < 4; c++){
            if (c > 0){
                s += ", ";
            }
            s += fnum(m[r][c], 5);
        }
        s += "]";
    }
    s += "\n    }";
    return s;
}

} //  anonymous namespace




CaptureCardCalibration_Descriptor::CaptureCardCalibration_Descriptor()
    : GameConsole::ConsoleProgramDescriptor(
        "NintendoSwitch:CaptureCardCalibration",
        "Nintendo Switch", "Capture Card Calibration",
        "",
        "Identify the capture card from a screenshot of the Switch color picker grid.",
        PanelQualityStanding::DEVELOPER,
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::ENABLE_COMMANDS
    )
{}

CaptureCardCalibration::CaptureCardCalibration()
    : THRESHOLD("<b>Match threshold (color levels):</b>", LockMode::LOCK_WHILE_RUNNING, 5.0, 0.0, 100.0)
{
    PA_ADD_OPTION(THRESHOLD);
}

void CaptureCardCalibration::program(GameConsole::ConsoleProgramEnvironment& env, CancellableScope& scope){
    //  Grab a frame from the capture card.
    VideoSnapshot snapshot = env.console().video().snapshot_latest_blocking();
    if (!snapshot.frame || snapshot.frame->width() == 0 || snapshot.frame->height() == 0){
        env.log(
            "No video frame available. Connect a capture card and put the Switch on the user-icon "
            "'Edit Background' color picker page, then run again.",
            COLOR_RED
        );
        return;
    }
    const ImageViewRGB32& image = *snapshot.frame;

    //  Load the measured capture card profiles.
    const std::string filename = UNIT_TEST_RESOURCE_PATH() + "CaptureCardCalibration/color_profile_to_transform.json";
    std::vector<std::pair<std::string, ColorTransform3x4>> profiles;
    {
        const JsonValue json = load_json_file(filename);
        for (const auto& item : json.to_object_throw(filename)){
            const JsonArray& rows = item.second.to_array_throw(filename);
            if (rows.size() != 3){
                throw ParseException("The transform of \"" + item.first + "\" in " + filename + " must have 3 rows.");
            }
            ColorTransform3x4 m = {};
            for (size_t r = 0; r < 3; r++){
                const JsonArray& row = rows[r].to_array_throw(filename);
                if (row.size() != 4){
                    throw ParseException("Each row of the transform of \"" + item.first + "\" in " + filename + " must have 4 numbers.");
                }
                for (size_t c = 0; c < 4; c++){
                    m[r][c] = row[c].to_double_throw(filename);
                }
            }
            profiles.emplace_back(item.first, m);
        }
    }
    if (profiles.empty()){
        env.log("No capture card profiles found in " + filename + ".", COLOR_RED);
        return;
    }

    //  Try every (console, page) combination and keep the one whose colors best match a profile.
    //  A wrong layout samples across patch edges and the background, so its colors match no profile.
    struct Result{
        const Combo* combo;
        std::vector<Color3> captured;
        double max_stddev;
        std::string best_label;
        double best_err;
    };
    Result best;
    best.combo = &COMBOS[0];
    best.best_err = 1e300;
    for (const Combo& combo : COMBOS){
        std::vector<Color3> captured;
        double max_stddev = 0.0;
        sample_patches(image, *combo.layout, combo.up, captured, max_stddev);

        const Color3* reference = combo.up ? REFERENCE_COLORS : (REFERENCE_COLORS + 30);
        std::string combo_best_label;
        double combo_best_err = 1e300;
        for (const auto& profile : profiles){
            const double err = clip_aware_rms(profile.second, reference, captured.data(), captured.size());
            if (err < combo_best_err){
                combo_best_err = err;
                combo_best_label = profile.first;
            }
        }
        if (combo_best_err < best.best_err){
            best = Result{&combo, std::move(captured), max_stddev, combo_best_label, combo_best_err};
        }
    }

    const double threshold = THRESHOLD;
    const char* page = best.combo->up ? "Up" : "Down";
    if (best.max_stddev > FLATNESS_WARNING_STDDEV){
        env.log(
            "Warning: the sampled patches aren't flat (max stddev " + fnum(best.max_stddev, 1) +
            "). Make sure the frame is showing the color picker grid.",
            COLOR_RED
        );
    }

    if (best.best_err <= threshold){
        env.log(
            "Capture card: " + best.best_label +
            "  (clip-aware color error " + fnum(best.best_err, 2) + " levels, " + best.combo->console + " " + page + " rows)",
            COLOR_GREEN
        );
    }else{
        const Color3* reference = best.combo->up ? REFERENCE_COLORS : (REFERENCE_COLORS + 30);
        const ColorTransform3x4 raw = fit_raw_transform(reference, best.captured.data(), best.captured.size());
        env.log(
            "No profile matched (closest: " + best.best_label + ", clip-aware color error " +
            fnum(best.best_err, 2) + " levels > threshold " + fnum(threshold, 1) + ").",
            COLOR_YELLOW
        );
        std::string raw_msg =
            "Fitted raw transform captured = clip(M @ reference + b, 0, 255), as a new "
            "color_profile_to_transform.json entry:\n    \"";
        raw_msg += best.combo->console;
        raw_msg += "-Unknown\":";
        raw_msg += format_transform(raw);
        env.log(raw_msg, COLOR_YELLOW);
    }
}



}
}
