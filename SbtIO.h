#ifndef SBTIO_HH
#define SBTIO_HH

#include <string>
#include <vector>

#include <Rtypes.h>

class SbtIO {
 public:
  static bool match(const char *pattern, const char *candidate, int p=0, int c=0);
  static bool createPath(const std::string& path);
  static std::vector<std::string> generateFileList(const std::string& path, const std::string& patter);
  static bool searchConfigFile(std::string& fileName);
  static std::string expandPath(const std::string& path);
 private:
  static int _mkpath(const char *path, mode_t mode);
  static int _do_mkdir(const char *path, mode_t mode);
  static void _generateConfigFileSearchPaths();
  static void _autoExpandEnvironmentVariables(std::string& text);

  static std::vector<std::string> _configFileSearchPaths;

  ClassDef(SbtIO, 1);
};

#endif
