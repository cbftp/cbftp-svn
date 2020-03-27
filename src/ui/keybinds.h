#pragma once

#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>

enum GenericKeyActions {
  KEYACTION_NONE = 100,
  KEYACTION_INFO,
  KEYACTION_ENTER,
  KEYACTION_BACK_CANCEL,
  KEYACTION_DONE,
  KEYACTION_DELETE,
  KEYACTION_0,
  KEYACTION_1,
  KEYACTION_2,
  KEYACTION_3,
  KEYACTION_4,
  KEYACTION_5,
  KEYACTION_6,
  KEYACTION_7,
  KEYACTION_8,
  KEYACTION_9,
  KEYACTION_KEYBINDS,
  KEYACTION_UP,
  KEYACTION_DOWN,
  KEYACTION_LEFT,
  KEYACTION_RIGHT,
  KEYACTION_PREVIOUS_PAGE,
  KEYACTION_NEXT_PAGE,
  KEYACTION_RESET,
  KEYACTION_ABORT,
  KEYACTION_TOP,
  KEYACTION_BOTTOM,
  KEYACTION_HIGHLIGHT_LINE,
  KEYACTION_TRANSFERS,
  KEYACTION_SKIPLIST,
  KEYACTION_FILTER,
  KEYACTION_FILTER_REGEX,
  KEYACTION_TOGGLE_ALL,
  KEYACTION_QUICK_JUMP,
  KEYACTION_CLOSE
};

enum GenericKeyScope {
  KEYSCOPE_ALL = 42
};

struct KeyRepr {
  std::string repr;
  unsigned int wch;
};

class KeyBinds {
public:
  KeyBinds(const std::string& name);
  KeyBinds(const KeyBinds& other);
  void addBind(int key, int keyaction, const std::string& description, int scope = KEYSCOPE_ALL);
  void addScope(int scope, const std::string& description);
  void customBind(int keyaction, int scope, int newkey);
  void resetBind(int keyaction, int scope);
  void unbind(int keyaction, int scope);
  void resetAll();
  int getKeyAction(int key, int scope = KEYSCOPE_ALL) const;
  std::string getName() const;
  typedef std::pair<int, int> KeyAndScope;
  typedef std::pair<int, int> ActionAndScope;
  struct KeyData {
    int keyaction;
    int scope;
    std::string description;
    unsigned int originalkey;
    unsigned int configuredkey;
  };
  std::list<KeyData> getBindsForScope(int scope) const;
  std::map<int, std::string>::const_iterator scopesBegin() const;
  std::map<int, std::string>::const_iterator scopesEnd() const;
  std::string getLegendSummary(int scope = KEYSCOPE_ALL) const;
  static KeyRepr getKeyRepr(unsigned int key);
  std::list<KeyData>::const_iterator begin() const;
  std::list<KeyData>::const_iterator end() const;
  bool hasExtraScopes() const;
  void disallowKeybinds();
  void useAlternateKeybindsButton();
private:
  void generateLegendSummaries();
  std::string name;
  std::map<KeyAndScope, std::list<KeyData>::iterator> keybinds;
  std::list<KeyData> keydata;
  std::map<int, std::string> scopes;
  std::set<ActionAndScope> actions;
  std::map<int, std::string> legendsummaries;
  bool allowkeybinds;
  bool alternatebutton;
};
