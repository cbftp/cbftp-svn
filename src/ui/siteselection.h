#pragma once

#include <list>
#include <memory>
#include <string>

class Site;

void fillPreselectionList(const std::string& preselectstr, std::list<std::shared_ptr<Site>>* list);
