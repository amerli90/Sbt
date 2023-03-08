#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <cerrno>
#include <regex>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "SbtIO.h"

ClassImp(SbtIO);

std::vector<std::string> SbtIO::_configFileSearchPaths;

bool SbtIO::match(const char* pattern, const char* candidate, int p, int c) {
  if (pattern[p] == '\0') {
    return candidate[c] == '\0';
  } else if (pattern[p] == '*') {
    for (; candidate[c] != '\0'; c++) {
      if (match(pattern, candidate, p + 1, c)) return true;
    }
    return match(pattern, candidate, p + 1, c);
  } else if (pattern[p] != '?' && pattern[p] != candidate[c]) {
    return false;
  } else {
    return match(pattern, candidate, p + 1, c + 1);
  }
}

std::vector<std::string> SbtIO::generateFileList(const std::string& path, const std::string& pattern) {
  std::vector<std::string> fileNameList;
  std::cout << "Creating the list of files..." << std::endl <<
  "Searching in '" << path << "' with file pattern '" << pattern << "'" << std::endl;
  DIR* dirp = opendir(path.c_str());
  if (!dirp) {
    std::cout << "Could not read directory '" << path << "'" << std::endl;
    return fileNameList;
  }
  dirent* dp = nullptr;
  while ((dp = readdir(dirp))) {
    if (match(pattern.c_str(), dp->d_name)) {
      std::stringstream fullName;
      fullName << path << "/" << dp->d_name;
      fileNameList.push_back(fullName.str());
      std::cout << "Adding '" << fullName.str() << "'" << std::endl;
    }
  }
  closedir(dirp);
  std::sort(fileNameList.begin(), fileNameList.end());
  return fileNameList;
}

bool SbtIO::createPath(const std::string& path)
{
  struct stat r;
  stat(path.c_str(), &r);
  if (!S_ISDIR(r.st_mode)) {
    std::cout << "Creating directory " << path << std::endl;
    _mkpath(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  }
  stat(path.c_str(), &r);
  return S_ISDIR(r.st_mode);
}

/**
** mkpath - ensure all directories in path exist
** Algorithm takes the pessimistic view and works top-down to ensure
** each directory in path exists, rather than optimistically creating
** the last element and working backwards.
*/
int SbtIO::_mkpath(const char *path, mode_t mode)
{
  char *sp = nullptr;
  int status = 0;
  char *copypath = strdup(path);
  char *pp = copypath;

  while (status == 0 && (sp = strchr(pp, '/')) != 0) {
    if (sp != pp) {
      /* Neither root nor double slash in path */
      *sp = '\0';
      status = _do_mkdir(copypath, mode);
      *sp = '/';
    }
    pp = sp + 1;
  }
  if (status == 0) status = _do_mkdir(path, mode);
  free(copypath);
  return (status);
}

int SbtIO::_do_mkdir(const char *path, mode_t mode)
{
  struct stat st;
  int status = 0;

  if (stat(path, &st) != 0) {
    /* Directory does not exist. EEXIST for race condition */
    if (mkdir(path, mode) != 0 && errno != EEXIST) status = -1;
  }
  else if (!S_ISDIR(st.st_mode)) {
    errno = ENOTDIR;
    status = -1;
  }

  return(status);
}

bool SbtIO::searchConfigFile(std::string& fileName) {
  if (_configFileSearchPaths.empty()) _generateConfigFileSearchPaths();
  struct stat buffer;
  for (const auto& searchPath : _configFileSearchPaths) {
    std::stringstream fullFileName;
    fullFileName << searchPath << "/" << fileName;
    if ((stat(fullFileName.str().c_str(), &buffer) == 0)) {
      // file exists
      fileName = fullFileName.str();
      return true;
    }
  }
  std::cout << "Could not find file '" << fileName << "'." << std::endl <<
  "The search was performed in: " << std::endl;
  for (auto path : _configFileSearchPaths) std::cout << path << std::endl;
  return false;
}

void SbtIO::_generateConfigFileSearchPaths() {
  _configFileSearchPaths.clear();
  _configFileSearchPaths.push_back(".");
  char* sbtConfigPath = std::getenv("SBT_CONFIG_PATH");
  if (sbtConfigPath) {
    char* token = std::strtok(sbtConfigPath, ":");
    while (token != nullptr) {
      _configFileSearchPaths.push_back(token);
      token = std::strtok(nullptr, ":");
    }
  }
}

// Update the input string.
void SbtIO::_autoExpandEnvironmentVariables(std::string& text ) {
    static std::regex env( "\\$\\{([^}]+)\\}" );
    std::smatch match;
    while ( std::regex_search( text, match, env ) ) {
        const char * s = getenv( match[1].str().c_str() );
        const std::string var( s == NULL ? "" : s );
        text.replace( match[0].first, match[0].second, var );
    }
}

// Leave input alone and return new string.
std::string SbtIO::expandPath(const std::string& path) {
    std::string text = path;
    _autoExpandEnvironmentVariables(text);
    return text;
}
