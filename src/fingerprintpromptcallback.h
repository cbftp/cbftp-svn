#pragma once

#include <string>

class FingerprintPromptCallback {
public:
  virtual ~FingerprintPromptCallback() = default;
  
  virtual void fingerprintPromptRequired(void* logic, 
                                        int connid,
                                        const std::string& sitename,
                                        const std::string& oldfingerprint,
                                        const std::string& newfingerprint) = 0;
};