"""
This replaces text for all files in the given folder.
Run the find_replace() function, with the globpath parameter as well as the function parameter.
    e.g. find_replace(globpath, replace_ssf_press_left_joystick)
To set the folder, update `globpath`. You can either use a path relative to this file, or an absolute path.
The function parameter should take a given file's contents, make the replacement, then return the new contents.
This function will be applied recursively on all child files.

The examples given below use re.sub to do the replacement.
re.sub() has 3 parameters:
1: the regex pattern that finds the selected strings.
2: the function that takes in matches (as per the regex pattern) and returns the desired replacement string
3: the old content

"""



import glob
import fileinput
import re
import tempfile
from pathlib import Path
import os



# look through each file, line by line (for very large files that you can't load into RAM)
# write the changes to a temp file, then replace the original file
def find_replace_line_by_line(top_dir, replace_content):
    for filepath in glob.iglob(top_dir, recursive=True):
        path = Path(filepath)
        if not path.is_file():
            continue

        # Use delete=False so the file persists after close()
        # Use 'dir=path.parent' to force the temp file onto the SAME DRIVE as the original. This prevents the "different disk drive" error.
        with tempfile.NamedTemporaryFile('w', encoding='utf-8', dir=path.parent, delete=False, newline='') as tf:
            temp_name = tf.name
            with open(path, 'r', encoding='utf-8', newline='') as f:
                for line in f:
                    new_line = replace_content(line)  # if no replacement, new_line is the same as line
                    tf.write(new_line)
                    # if new_line != line:
                    #     print(line)
                    #     print(new_line)



        # 2. FILE IS NOW CLOSED (Exit 'with' block)
        # On Windows, you MUST close the file before os.replace can access it
        # Replace the original file with the new one atomically
        try:
            # if os.path.exists(temp_name):
            #     os.remove(temp_name)
            os.replace(temp_name, path)
        except Exception as e:
            # 3. Clean up if the replacement fails
            if os.path.exists(temp_name):
                os.remove(temp_name)
            print(f"Error updating {path}: {e}")
            raise






def replace_pbf_left_joystick(old_content: str) -> str:
    # Regex pattern:
    # 1. Matches "pbf_move_left_joystick_old("
    function_name = r"pbf_move_left_joystick_old\("
    # 2. Group 1: Captures first 3 arguments: (.*?, \d+, \d+)
    group_1 = r"(.*?,\s*\d+,\s*\d+)"
    # 3. Group 2, 3: Captures last 2 numeric durations: (\d+), (\d+)
    group_2 = r"(\d+)"
    group_3 = r"(\d+)"
    pattern = function_name + group_1 + r",\s*" + group_2 + r",\s*" + group_3 + r"\);"

    def replace_joystick_ticks_with_milliseconds(match):
        # e.g. pbf_move_left_joystick_old(context, 128, 0, 10, 10);
        # match.group(1) is "context, 128, 0"
        # match.group(2) is "10" (first duration)
        # match.group(3) is "10" (second duration)
        prefix = match.group(1)
        dur1 = int(match.group(2)) * 8
        dur2 = int(match.group(3)) * 8

        return f"pbf_move_left_joystick_old({prefix}, {dur1}ms, {dur2}ms);"

    new_content: str = re.sub(pattern, replace_joystick_ticks_with_milliseconds, old_content)

    return new_content

def replace_pbf_left_joystick2(old_content: str) -> str:
    # Regex pattern:
    # 1. Matches "pbf_move_left_joystick_old("
    function_name = r"pbf_move_left_joystick_old1\("
    # 2. Group 1: Captures first 3 arguments: (.*?, .*?, .*?)
    group_1 = r"(.*?,\s*.*?,\s*.*?)"
    # 3. Group 2, 3: Captures last 2 numeric durations: (\d+), (\d+)
    group_2 = r"(\d+)"
    group_3 = r"(\d+)"
    pattern = function_name + group_1 + r",\s*" + group_2 + r",\s*" + group_3 + r"\);"

    def replace_joystick_ticks_with_milliseconds(match):
        # e.g. pbf_move_left_joystick_old(context, 128, 0, 10, 10);
        # match.group(1) is "context, 128, 0"
        # match.group(2) is "10" (first duration)
        # match.group(3) is "10" (second duration)
        prefix = match.group(1)
        dur1 = int(match.group(2)) * 8
        dur2 = int(match.group(3)) * 8

        return f"pbf_move_left_joystick_old({prefix}, {dur1}ms, {dur2}ms);"

    new_content: str = re.sub(pattern, replace_joystick_ticks_with_milliseconds, old_content)

    return new_content


def replace_pbf_left_joystick3(old_content: str) -> str:
    # Regex pattern:
    # 1. Matches "pbf_move_left_joystick_old("
    function_name = r"pbf_move_left_joystick_old1\("
    # 2. Group 1: Captures first 4 arguments: (.*?, .*?, .*?, .*?)
    group_1 = r"(.*?,\s*.*?,\s*.*?,\s*.*?)"
    # 3. Group 2, 3: Captures last numeric duration: (\d+)
    group_2 = r"(\d+)"
    pattern = function_name + group_1 + r",\s*" + group_2 + r"\);"

    def replace_joystick_ticks_with_milliseconds(match):
        # e.g. pbf_move_left_joystick_old(context, 128, 0, 10, 10);
        # match.group(1) is "context, 128, 0"
        # match.group(2) is "10" (first duration)
        # match.group(3) is "10" (second duration)
        prefix = match.group(1)
        dur1 = int(match.group(2)) * 8

        return f"pbf_move_left_joystick_old1({prefix}, {dur1}ms);"

    new_content: str = re.sub(pattern, replace_joystick_ticks_with_milliseconds, old_content)

    return new_content

def replace_mash_button(old_content: str) -> str:
    # Regex pattern:
    # 1. Matches "pbf_mash_button_old("
    function_name = r"pbf_mash_button_old\("
    # 2. Group 1: Captures first 2 arguments: (.*?, .*?)
    group_1 = r"(.*?,\s*.*?)"
    # 3. Group 2: Captures last numeric duration: (\d+)
    group_2 = r"(\d+)"
    pattern = function_name + group_1 + r",\s*" + group_2 + r"\);"


    def replace_mash_button_ticks_with_milliseconds(match):
        # e.g. pbf_mash_button_old(context, BUTTON_A, 10);
        # match.group(1) is "context, BUTTON_A"
        # match.group(2) is "10" (first duration)
        prefix = match.group(1)
        dur1 = int(match.group(2)) * 8

        return f"pbf_mash_button({prefix}, {dur1}ms);"

    new_content: str = re.sub(pattern, replace_mash_button_ticks_with_milliseconds, old_content)

    return new_content



def replace_pbf_wait(old_content: str) -> str:
    # Regex pattern:
    # 1. Matches "pbf_wait_old("
    function_name = r"pbf_wait_old\("
    # 2. Group 1: Captures first argument: (.*?)
    group_1 = r"(.*?)"
    # 3. Group 2: Captures last numeric duration: (\d+)
    group_2 = r"(\d+)"
    pattern = function_name + group_1 + r",\s*" + group_2 + r"\);"


    def replace_pbf_wait_ticks_with_milliseconds(match):
        # e.g. pbf_wait_old(context, BUTTON_A, 10);
        # groups are defined as per the regex pattern above
        # match.group(1) is "context, BUTTON_A"
        # match.group(2) is "10" (first duration)
        prefix = match.group(1)
        dur1 = int(match.group(2)) * 8

        return f"pbf_wait({prefix}, {dur1}ms);"

    new_content: str = re.sub(pattern, replace_pbf_wait_ticks_with_milliseconds, old_content)

    return new_content


def replace_pbf_button(old_content: str) -> str:
    # Regex pattern:
    # 1. Matches "pbf_press_button_old("
    function_name = r"pbf_press_button_old\("
    # 2. Group 1: Captures first 2 arguments: (.*?, .*?)
    group_1 = r"(.*?,\s*.*?)"
    # 3. Group 2, 3: Captures last 2 numeric durations: (\d+), (\d+)
    group_2 = r"(\d+)"
    group_3 = r"(\d+)"
    pattern = function_name + group_1 + r",\s*" + group_2 + r",\s*" + group_3 + r"\);"

    def replace_button_ticks_with_milliseconds(match):
        # e.g. pbf_press_button_old(context, BUTTON_A, 10, 20);
        # match.group(1) is "context, BUTTON_A,"
        # match.group(2) is "10" (first duration)
        # match.group(3) is "20" (second duration)
        prefix = match.group(1)
        dur1 = int(match.group(2)) * 8
        dur2 = int(match.group(3)) * 8

        return f"pbf_press_button({prefix}, {dur1}ms, {dur2}ms);"

    new_content: str = re.sub(pattern, replace_button_ticks_with_milliseconds, old_content)

    return new_content


def replace_pbf_dpad(old_content: str) -> str:
    # Regex pattern:
    # 1. Matches "pbf_press_button_old("
    function_name = r"pbf_press_dpad_old\("
    # 2. Group 1: Captures first 2 arguments: (.*?, .*?)
    group_1 = r"(.*?,\s*.*?)"
    # 3. Group 2, 3: Captures last 2 numeric durations: (\d+), (\d+)
    group_2 = r"(\d+)"
    group_3 = r"(\d+)"
    pattern = function_name + group_1 + r",\s*" + group_2 + r",\s*" + group_3 + r"\);"

    def replace_dpad_ticks_with_milliseconds(match):
        # e.g. pbf_press_dpad_old(context, DPAD_UP, 10, 20);
        # match.group(1) is "context, BUTTON_A,"
        # match.group(2) is "10" (first duration)
        # match.group(3) is "20" (second duration)
        prefix = match.group(1)
        dur1 = int(match.group(2)) * 8
        dur2 = int(match.group(3)) * 8

        return f"pbf_press_dpad({prefix}, {dur1}ms, {dur2}ms);"

    new_content: str = re.sub(pattern, replace_dpad_ticks_with_milliseconds, old_content)

    return new_content

def replace_pbf_left_joystick4(old_content: str) -> str:
    # Regex pattern:
    # 1. Matches "pbf_move_left_joystick_old("
    function_name = r"pbf_move_left_joystick_old\("
    # 2. Group 1: Captures first argument: (.*?)
    group_1 = r"(.*?)"
    # 3. Group 2, 3: Captures next 2 numeric x, y: (\d+), (\d+)
    group_2 = r"(\d+)"
    group_3 = r"(\d+)"
    # 4. Group 4 captures hold, delay duration: (\d+ms, \d+ms)
    group_4 = r"(.*?,\s*.*?)"
    pattern = function_name + group_1 + r",\s*" + group_2 + r",\s*" + group_3 +  r",\s*" + group_4 + r"\);"

    def replace_joystick_u8_to_float(match):
        # e.g. pbf_move_left_joystick_old(context, 128, 0, 160ms, 160ms);
        # match.group(1) is "context"
        # match.group(2) is 128 (x)
        # match.group(3) is 0 (y)
        # match.group(4) is 160ms, 160ms

        # convert to: pbf_move_left_joystick(context, {0, +1}, 160ms, 160ms);
        prefix = match.group(1)
        x_float = linear_u8_to_float(int(match.group(2)))
        y_float = -linear_u8_to_float(int(match.group(3)))

        x_string = linear_float_to_string(x_float, 3)
        y_string = linear_float_to_string(y_float, 3)

        group_4 = match.group(4)

        return f"pbf_move_left_joystick({prefix}, {{{x_string}, {y_string}}}, {group_4});"

    new_content: str = re.sub(pattern, replace_joystick_u8_to_float, old_content)

    return new_content

def replace_pbf_left_joystick_flying_trial(old_content: str) -> str:
    # Regex pattern:
    # 1. Matches "pbf_move_left_joystick_old("
    function_name = r"pbf_move_left_joystick_old\("
    # 2. Group 1: Captures first argument: (.*?)
    group_1 = r"(.*?)"
    # 3. Group 2, 3: Captures next 2 numeric x, y: (\d+), (\d+)
    group_2 = r"(\d+)"
    group_3 = r"(-?\d+)"
    # 4. Group 4 captures hold, delay duration: (\d+ms, \d+ms)
    group_4 = r"(.*?,\s*.*?)"
    pattern = function_name + group_1 + r",\s*" + group_2 + r",\s*get_final_y_axis\(\s*" + group_3 +  r"\),\s*" + group_4 + r"\);"

    def replace_joystick_u8_to_float(match):
        # e.g. pbf_move_left_joystick_old(context, 128, get_final_y_axis( 0), 160ms, 160ms);
        # match.group(1) is "context"
        # match.group(2) is 128 (x)
        # match.group(3) is 0 (y)
        # match.group(4) is 160ms, 160ms

        # convert to: pbf_move_left_joystick(context, {0, +1}, 160ms, 160ms);
        prefix = match.group(1)
        x_float = linear_u8_to_float(int(match.group(2)))
        y_magnitude = -float(match.group(3))/128.0

        x_string = linear_float_to_string(x_float, 3)
        y_string = linear_float_to_string(y_magnitude, 3)

        group_4 = match.group(4)

        return f"pbf_move_left_joystick({prefix}, {{{x_string}, get_final_y_axis({y_string})}}, {group_4});"

    new_content: str = re.sub(pattern, replace_joystick_u8_to_float, old_content)

    return new_content


def replace_move_cursor_struct(old_content: str) -> str:
    # Regex pattern:
    # 1. Matches "pbf_move_left_joystick_old("
    group_0 = r"\{ZoomChange::"
    # 2. Group 1: Captures ZoomChange type: (.*?). e.g. KEEP_ZOOM
    group_1 = r"(.*?)"
    # 3. Group 2, 3: Captures next 2 numeric x, y: e.g. 0, 10
    group_2 = r"(\d+)"
    group_3 = r"(\d+)"
    # 4. Group 4 captures move duration: e.g. 100ms
    group_4 = r"(\d+ms)"
    pattern = group_0 + group_1 + r",\s*" + group_2 + r",\s*"  + group_3 +  r",\s*"  + group_4 + r"\}"

    def replace_joystick_u8_to_float(match):
        # e.g. {ZoomChange::KEEP_ZOOM, 0, 10, 100ms}
        # match.group(1) is "KEEP_ZOOM"
        # match.group(2) is 0 (x)
        # match.group(3) is 10 (y)
        # match.group(4) is 100ms

        zoom_change = match.group(1)
        x_float = linear_u8_to_float(int(match.group(2)))
        y_float = -linear_u8_to_float(int(match.group(3)))

        x_string = linear_float_to_string(x_float, 3)
        y_string = linear_float_to_string(y_float, 3)

        group_4 = match.group(4)

        return f"{{ZoomChange::{zoom_change}, {x_string}, {y_string}, {group_4}}}"

    new_content: str = re.sub(pattern, replace_joystick_u8_to_float, old_content)

    return new_content

def replace_walk_forward_until_dialog(old_content: str) -> str:
    # Regex pattern:
    # 1. Matches "walk_forward_until_dialog("
    function_name = r"walk_forward_until_dialog\("
    # 2. Group 1: Captures first 5 arguments: e.g. env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 60000ms,
    group_1 = r"(.*?,\s*.*?,\s*.*?,\s*.*?,\s*.*?)"
    # 3. Group 2, 3: Captures last 2 numeric durations: e.g. 0, 128
    group_2 = r"(\d+)"
    group_3 = r"(\d+)"
    pattern = function_name + group_1 + r",\s*" + group_2 +  r",\s*" + group_3 + r"\);"


    def replace_joystick_u8_to_float(match):
        # e.g. walk_forward_until_dialog(env.program_info(), env.console, context, NavigationMovementMode::DIRECTIONAL_ONLY, 60000ms, 0, 128);
        # groups are defined as per the regex pattern above
        # match.group(1) is "context, BUTTON_A"
        # match.group(2) is "10" (first duration)
        prefix = match.group(1)
        x_float = linear_u8_to_float(int(match.group(2)))
        y_float = -linear_u8_to_float(int(match.group(3)))

        x_string = linear_float_to_string(x_float, 3)
        y_string = linear_float_to_string(y_float, 3)

        return f"walk_forward_until_dialog({prefix}, {x_string}, {y_string});"

    new_content: str = re.sub(pattern, replace_joystick_u8_to_float, old_content)

    return new_content

def replace_realign_player(old_content: str) -> str:
    # Regex pattern:
    # 1. Matches "realign_player("
    function_name = r"realign_player\("
    # 2. Group 1: Captures first 4 arguments: e.g. env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER,
    group_1 = r"(.*?,\s*.*?,\s*.*?,\s*.*?)"
    # 3. Group 2, 3: Captures x, y: e.g. 0, 128
    group_2 = r"(\d+)"
    group_3 = r"(\d+)"
    # 4. Group 4: Captures duration: e.g. 80 ticks
    group_4 = r"(\d+)"
    pattern = function_name + group_1 + r",\s*" + group_2 +  r",\s*" + group_3 +  r",\s*" + group_4 + r"\);"


    def replace_joystick_u8_to_float(match):
        # e.g. realign_player(env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER, 128, 0, 80);
        # groups are defined as per the regex pattern above
        # match.group(1) is "env.program_info(), env.console, context, PlayerRealignMode::REALIGN_NEW_MARKER"
        # match.group(2) is "128" (first duration)
        # match.group(3) is "0" (first duration)
        # match.group(4) is "80" (first duration)
        prefix = match.group(1)
        x_float = linear_u8_to_float(int(match.group(2)))
        y_float = -linear_u8_to_float(int(match.group(3)))
        duration = int(match.group(4)) * 8

        x_string = linear_float_to_string(x_float, 3)
        y_string = linear_float_to_string(y_float, 3)

        return f"realign_player({prefix}, {x_string}, {y_string}, {duration}ms);"

    new_content: str = re.sub(pattern, replace_joystick_u8_to_float, old_content)

    return new_content

def replace_map_move_cursor_fly(old_content: str) -> str:
    # Regex pattern:
    # 1. Matches "map_move_cursor_fly("
    function_name = r"map_move_cursor_fly\("
    # 2. Group 1: Captures first 3 arguments: e.g. info, stream, context
    group_1 = r"(.*?,\s*.*?,\s*.*?)"
    # 3. Group 2, 3: Captures x, y: e.g. 0, 128
    group_2 = r"(\d+)"
    group_3 = r"(\d+)"
    # 4. Group 4: Captures rest of arguments: e.g. 1840ms, 160ms, "Polar Rest Area"
    group_4 = r"(.*?)"
    pattern = function_name + group_1 + r",\s*" + group_2 +  r",\s*" + group_3 +  r",\s*" + group_4 + r"\);"


    def replace_joystick_u8_to_float(match):
        # e.g. map_move_cursor_fly(info, stream, context, 75, 0, 1840ms, 160ms, "Polar Rest Area");
        # groups are defined as per the regex pattern above
        prefix = match.group(1)
        x_float = linear_u8_to_float(int(match.group(2)))
        y_float = -linear_u8_to_float(int(match.group(3)))
        suffix = match.group(4)

        x_string = linear_float_to_string(x_float, 3)
        y_string = linear_float_to_string(y_float, 3)

        return f"map_move_cursor_fly({prefix}, {x_string}, {y_string}, {suffix});"

    new_content: str = re.sub(pattern, replace_joystick_u8_to_float, old_content)

    return new_content
    
def replace_overworld_navigation(old_content: str) -> str:
    # Regex pattern:
    # 1. Matches "overworld_navigation("
    function_name = r"overworld_navigation\("
    # 2. Group 1: Captures first 5 arguments: e.g. env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY
    group_1 = r"(.*?,\s*.*?,\s*.*?,\s*.*?,\s*.*?,\s*)"
    # 3. Group 2, 3: Captures x, y: e.g. 0, 128
    group_2 = r"(\d+)"
    group_3 = r"(\d+)"
    # 4. Group 4: Captures rest of arguments: e.g. 20, 20, true, true
    group_4 = r"(.*?)"
    pattern = function_name + group_1 + group_2 +  r",\s*" + group_3 +  r",\s*" + group_4 + r"\);"


    def replace_joystick_u8_to_float(match):
        # e.g. overworld_navigation(env.program_info(), env.console, context, NavigationStopCondition::STOP_DIALOG, NavigationMovementMode::DIRECTIONAL_ONLY, 128, 0, 20, 20, true, true);
        # groups are defined as per the regex pattern above
        prefix = match.group(1)
        x_float = linear_u8_to_float(int(match.group(2)))
        y_float = -linear_u8_to_float(int(match.group(3)))
        suffix = match.group(4)

        x_string = linear_float_to_string(x_float, 3)
        y_string = linear_float_to_string(y_float, 3)

        return f"overworld_navigation({prefix}{x_string}, {y_string}, {suffix});"

    new_content: str = re.sub(pattern, replace_joystick_u8_to_float, old_content)

    return new_content


def replace_ssf_press_left_joystick(old_content: str) -> str:
    # Regex pattern:
    # 1. Matches "overworld_navigation("
    function_name = r"ssf_press_left_joystick_old\("
    # 2. Group 1: Captures first argument: e.g. context, STICK_MAX, STICK_MAX
    group_1 = r"(.*?,\s*.*?,\s*.*?)"
    # 3. Group 2, 3: Captures last 2 numeric durations: e.g. 0, 128
    group_2 = r"(\d+)"
    group_3 = r"(\d+)"
    pattern = function_name + group_1 +  r",\s*" + group_2 +  r",\s*" + group_3 +  r"\);"


    def replace_ticks_to_milliseconds(match):
        # e.g. ssf_press_left_joystick_old(context, STICK_MAX, STICK_MAX, 400ms, 400ms);
        # groups are defined as per the regex pattern above
        prefix = match.group(1)
        dur1 = int(match.group(2)) * 8
        dur2 = int(match.group(3)) * 8

        return f"ssf_press_left_joystick_old({prefix}, {dur1}ms, {dur2}ms);"

    new_content: str = re.sub(pattern, replace_ticks_to_milliseconds, old_content)

    return new_content

def replace_ssf_press_left_joystick2(old_content: str) -> str:
    # Regex pattern:
    # 1. Matches "overworld_navigation("
    function_name = r"ssf_press_left_joystick_old\("
    # 2. Group 1: Captures first argument: e.g. context
    group_1 = r"(.*?)"
    # 3. Group 2, 3: Captures x, y: e.g. STICK_MAX, STICK_MAX
    group_2 = r"(.*?)"
    group_3 = r"(.*?)"
    # 4. Group 4: Captures rest of arguments: e.g. 400ms, 400ms
    group_4 = r"(.*?)"
    pattern = function_name + group_1 +  r",\s*" + group_2 +  r",\s*" + group_3 +  r",\s*" + group_4 + r"\);"


    def replace_joystick_u8_to_float(match):
        # e.g. ssf_press_left_joystick_old(context, STICK_MAX, STICK_MAX, 400ms, 400ms);
        # groups are defined as per the regex pattern above
        prefix = match.group(1)
        x = match.group(2)
        y = match.group(3)
        # print(match)
        x_string = ""
        y_string = ""
        try:
            x_float = -10
            if x == "STICK_MAX":
                x_float = 1
            elif x == "STICK_CENTER":
                x_float = 0
            elif x == "STICK_MIN":
                x_float = -1
            else:
                x_float = linear_u8_to_float(int(x))

            x_string = linear_float_to_string(x_float, 3)
        except ValueError:
            x_string = x
        
        try:
            y_float = -10
            if y == "STICK_MIN":
                y_float = 1
            elif y == "STICK_CENTER":
                y_float = 0
            elif y == "STICK_MAX":
                y_float = -1
            else:
                y_float = -linear_u8_to_float(int(y))

            y_string = linear_float_to_string(y_float, 3)
        except ValueError:
            y_string = y


        suffix = match.group(4)

        return f"ssf_press_left_joystick({prefix}, {{{x_string}, {y_string}}}, {suffix});"

    new_content: str = re.sub(pattern, replace_joystick_u8_to_float, old_content)

    return new_content

def linear_float_to_string(x: float, decimal_places: int) -> str:
    if (x == 0):
        return "0"
    if (x == 1):
        return "+1"
    if (x == -1):
        return "-1"
    elif (x > 0):
        return "+" + str(round(x, decimal_places))
    else:  #(x < 0):
        return str(round(x, decimal_places))

def linear_u8_to_float(x: int) -> float:
    if (x == 128):
        return 0
    elif (x == 64):
        return -0.5
    elif (x == 192):
        return 0.5
    elif (x < 128):
        return (x - 128) * (1. / 128)
    else: # x > 128
        return (x - 128) * (1. / 127)


# top_dir: top level directory, within which it will be recursively searched for .cpp files
# replace_content: the function that returns the new file content that will replace the old
def find_replace(top_dir, replace_content):
    for filepath in glob.iglob(top_dir, recursive=True):
        path = Path(filepath)
        if not path.is_file():
            continue

        # 1. Read the file
        # we need newline='' so that line endings are preserved
        with open(filepath, 'r', encoding='utf-8', newline='') as f:
            content = f.read()

        # 2. Perform the global substitution
        updated_content = replace_content(content)

        if (updated_content == content): # don't overwrite the file if the file content is the same
            continue

        # 3. Overwrite the file
        with open(filepath, 'w', encoding='utf-8', newline='') as f:
            f.write(updated_content)



# globpath = '../Pokemon-Arduino-Source/SerialPrograms/Source/**/*.cpp'
globpath = '../Pokemon-Arduino-Source/SerialPrograms/Source/**/*.*'

# find_replace(globpath, replace_pbf_left_joystick)
# find_replace_line_by_line(globpath, replace_pbf_left_joystick)
# find_replace(globpath, replace_mash_button)
# find_replace(globpath, replace_pbf_wait)
# find_replace(globpath, replace_pbf_button)
# find_replace(globpath, replace_pbf_dpad)
find_replace(globpath, replace_ssf_press_left_joystick2)