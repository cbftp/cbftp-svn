#include "keybinds.h"

#include <cassert>

#include "../util.h"

#define KEY_UNSET 4711

KeyRepr KeyBinds::getKeyRepr(unsigned int key) {
  KeyRepr repr;
  repr.wch = key;
  switch (repr.wch) {
    case 0:
      repr.repr = "Ctrl+Space";
      return repr;
    case 9:
      repr.repr = "Tab";
      return repr;
    case 10:
      repr.repr = "Enter";
      return repr;
    case 27:
      repr.repr = "Escape";
      return repr;
    case 32:
      repr.repr = "Space";
      return repr;
    case 258:
      repr.repr = "Down";
      return repr;
    case 259:
      repr.repr = "Up";
      return repr;
    case 260:
      repr.repr = "Left";
      return repr;
    case 261:
      repr.repr = "Right";
      return repr;
    case 262:
      repr.repr = "Home";
      return repr;
    case 263:
      repr.repr = "Backspace";
      return repr;
    case 330:
      repr.repr = "Delete";
      return repr;
    case 331:
      repr.repr = "Insert";
      return repr;
    case 336:
      repr.repr = "Shift+Down";
      return repr;
    case 337:
      repr.repr = "Shift+Up";
      return repr;
    case 338:
      repr.repr = "Page Down";
      return repr;
    case 339:
      repr.repr = "Page Up";
      return repr;
    case 353:
      repr.repr = "Shift+Tab";
      return repr;
    case 360:
      repr.repr = "End";
      return repr;
    case 383:
      repr.repr = "Shift+Delete";
      return repr;
    case 393:
      repr.repr = "Shift+Left";
      return repr;
    case 402:
      repr.repr = "Shift+Right";
      return repr;
    case 524:
      repr.repr = "Ctrl+Down";
      return repr;
    case 544:
      repr.repr = "Ctrl+Left";
      return repr;
    case 545:
      repr.repr = "Ctrl+Shift+Left";
      return repr;
    case 559:
      repr.repr = "Ctrl+Right";
      return repr;
    case 560:
      repr.repr = "Ctrl+Shift+Right";
      return repr;
    case 565:
      repr.repr = "Ctrl+Up";
      return repr;
    case KEY_UNSET:
      repr.repr = "None";
      return repr;
  }
  if (repr.wch >= 1 && repr.wch <= 26) {
    char ctrlkey = repr.wch + 96;
    repr.repr = std::string("Ctrl+") + ctrlkey;
  }
  if (repr.wch >= 265 && repr.wch <= 273) {
    char fkey = repr.wch - 216;
    repr.repr = std::string("F") + fkey;
  }
  if (repr.wch >= 274 && repr.wch <= 276) {
    char fkey = repr.wch - 226;
    repr.repr = std::string("F1") + fkey;
  }
  return repr;
}

KeyBinds::KeyBinds(const std::string& name) : name(name), allowkeybinds(true), alternatebutton(false) {
  addScope(KEYSCOPE_ALL, "Common");
}

KeyBinds::KeyBinds(const KeyBinds& other) : name(other.name), keydata(other.keydata),
    scopes(other.scopes), actions(other.actions), allowkeybinds(other.allowkeybinds)
{
  for (std::list<KeyData>::iterator it = keydata.begin(); it != keydata.end(); ++it) {
    KeyAndScope token(it->configuredkey, it->scope);
    keybinds[token] = it;
  }
}

void KeyBinds::addBind(int key, int keyaction, const std::string& description, int scope) {
  ActionAndScope action(keyaction, scope);
  assert(actions.find(action) == actions.end());
  assert(scopes.find(scope) != scopes.end());
  KeyAndScope token(key, scope);
  std::map<KeyAndScope, std::list<KeyData>::iterator>::const_iterator it = keybinds.find(token);
  assert(it == keybinds.end());
  actions.insert(action);
  KeyData keydat;
  keydat.keyaction = keyaction;
  keydat.description = description;
  keydat.originalkey = key;
  keydat.scope = scope;
  keydat.configuredkey = key;
  keydata.push_back(keydat);
  keybinds[token] = --keydata.end();
  generateLegendSummaries();
}

void KeyBinds::addScope(int scope, const std::string& description) {
  scopes[scope] = description;
}

void KeyBinds::customBind(int keyaction, int scope, int newkey) {
  for (std::list<KeyData>::iterator it = keydata.begin(); it != keydata.end(); ++it) {
    if (it->scope == scope && it->keyaction == keyaction) {
      keybinds.erase(KeyAndScope(it->configuredkey, scope));
      it->configuredkey = newkey;
      KeyAndScope token(newkey, scope);
      keybinds[token] = it;
      generateLegendSummaries();
      return;
    }
  }
}

void KeyBinds::resetBind(int keyaction, int scope) {
  for (std::list<KeyData>::iterator it = keydata.begin(); it != keydata.end(); ++it) {
    if (it->scope == scope && it->keyaction == keyaction) {
      if (it->configuredkey == it->originalkey) {
        return;
      }
      keybinds.erase(KeyAndScope(it->configuredkey, scope));
      it->configuredkey = it->originalkey;
      KeyAndScope token(it->originalkey, scope);
      keybinds[token] = it;
      generateLegendSummaries();
      return;
    }
  }
}

void KeyBinds::unbind(int keyaction, int scope) {
  for (std::list<KeyData>::iterator it = keydata.begin(); it != keydata.end(); ++it) {
    if (it->scope == scope && it->keyaction == keyaction) {
      keybinds.erase(KeyAndScope(it->configuredkey, scope));
      it->configuredkey = KEY_UNSET;
      generateLegendSummaries();
      return;
    }
  }
}

void KeyBinds::resetAll() {
  keybinds.clear();
  for (std::list<KeyData>::iterator it = keydata.begin(); it != keydata.end(); ++it) {
    it->configuredkey = it->originalkey;
    KeyAndScope token(it->originalkey, it->scope);
    keybinds[token] = it;
  }
  generateLegendSummaries();
}

int KeyBinds::getKeyAction(int key, int scope) const {
  if (key == 27) {
    return KEYACTION_BACK_CANCEL;
  }
  if (allowkeybinds && ((key == '?' && !alternatebutton) || (key == 276 && alternatebutton))) {
    return KEYACTION_KEYBINDS;
  }
  if (key == '1') {
    return KEYACTION_1;
  }
  if (key == '2') {
    return KEYACTION_2;
  }
  if (key == '3') {
    return KEYACTION_3;
  }
  if (key == '4') {
    return KEYACTION_4;
  }
  if (key == '5') {
    return KEYACTION_5;
  }
  if (key == '6') {
    return KEYACTION_6;
  }
  if (key == '7') {
    return KEYACTION_7;
  }
  if (key == '8') {
    return KEYACTION_8;
  }
  if (key == '9') {
    return KEYACTION_9;
  }
  KeyAndScope token(key, scope);
  auto it = keybinds.find(token);
  if (it == keybinds.end()) {
    if (token.second != KEYSCOPE_ALL) {
      token.second = KEYSCOPE_ALL;
      it = keybinds.find(token);
    }
  }
  if (it != keybinds.end()) {
    return it->second->keyaction;
  }
  return KEYACTION_NONE;
}

std::string KeyBinds::getName() const {
  return name;
}

std::list<KeyBinds::KeyData> KeyBinds::getBindsForScope(int scope) const {
  std::list<KeyBinds::KeyData> out;
  for (const KeyData& keydat : keydata) {
    if (keydat.scope == scope) {
      out.push_back(keydat);
    }
  }
  return out;
}

std::map<int, std::string>::const_iterator KeyBinds::scopesBegin() const {
  return scopes.begin();
}

std::map<int, std::string>::const_iterator KeyBinds::scopesEnd() const {
  return scopes.end();
}

std::string KeyBinds::getLegendSummary(int scope) const {
  auto it = legendsummaries.find(scope);
  if (it != legendsummaries.end()) {
    return it->second;
  }
  return "";
}

void KeyBinds::generateLegendSummaries() {
  for (const std::pair<int, std::string>& scope : scopes) {
    std::list<KeyBinds::KeyData> binds = getBindsForScope(scope.first);
    std::string summary;
    if (allowkeybinds) {
      summary += "[" + (alternatebutton ? getKeyRepr(276).repr : "?") + "] All keybinds";
    }
    for (const KeyBinds::KeyData& keydata : binds) {
      summary += summary.empty() ? "" : " - ";
      KeyRepr repr = getKeyRepr(keydata.configuredkey);
      std::string append = keydata.description;
      std::string key = repr.repr.empty() ? std::string() + static_cast<char>(repr.wch) : repr.repr;
      if (scope.first == KEYSCOPE_ALL && keydata.keyaction == KEYACTION_BACK_CANCEL && keydata.configuredkey != 27) {
        key = "Esc/" + key;
      }
      size_t pos = util::toLower(keydata.description).find(util::toLower(key));
      if (pos != std::string::npos) {
        append = append.substr(0, pos) + "[" + key + "]" + append.substr(pos + key.length());
      }
      else {
        append = "[" + key + "] " + append;
      }
      summary += append;
    }
    legendsummaries[scope.first] = summary;
  }
  std::string all = legendsummaries[KEYSCOPE_ALL];
  if (all.empty()) {
    return;
  }
  for (std::map<int, std::string>::iterator it = legendsummaries.begin(); it != legendsummaries.end(); ++it) {
    if (it->first != KEYSCOPE_ALL) {
      it->second += (!it->second.empty() ? " - " : "") + all;
    }
  }
}

std::list<KeyBinds::KeyData>::const_iterator KeyBinds::begin() const {
  return keydata.begin();
}

std::list<KeyBinds::KeyData>::const_iterator KeyBinds::end() const {
  return keydata.end();
}

bool KeyBinds::hasExtraScopes() const {
  return scopes.size() > 1;
}

void KeyBinds::disallowKeybinds() {
  allowkeybinds = false;
  generateLegendSummaries();
}

void KeyBinds::useAlternateKeybindsButton() {
  alternatebutton = true;
  generateLegendSummaries();
}
