class LibdppAT1015 < Formula
  desc "C++ Discord API Bot Library"
  homepage "https://github.com/brainboxdotcc/DPP"
  url "https://github.com/brainboxdotcc/DPP/releases/download/v10.1.5/DPP-10.1.5.tar.gz"
  sha256 "0446993c2bca5fc40882386804598b33652fc7ee466fa237f7846f2be0cb8a1e"
  license "Apache-2.0"

  depends_on "cmake" => :build
  depends_on xcode: ["12.0", :build]
  depends_on "libsodium"
  depends_on "openssl@3"
  depends_on "opus"
  depends_on "pkg-config"

  uses_from_macos "zlib"

  def install
    system "cmake", "-S", ".", "-B", "build", *std_cmake_args
    system "cmake", "--build", "build"
    system "cmake", "--install", "build"
  end

  test do
    (testpath/"test.cpp").write <<~EOS
      #include <dpp/dpp.h>

      int main() {
        dpp::cluster bot("invalid_token");

        bot.on_log(dpp::utility::cout_logger());

        try {
          bot.start(dpp::st_wait);
        }
        catch(dpp::invalid_token_exception& e) {
          std::cout << "Invalid token." << std::endl;
          return 0;
        }
        return 0;
      }
    EOS
    system ENV.cxx, "-std=c++17", "-L#{lib}", "-I#{include}", "test.cpp", "-o", "test", "-ldpp"
    assert_equal "Invalid token.", shell_output("./test").strip
  end
end
