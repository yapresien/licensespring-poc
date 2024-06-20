#pragma once

#include "SampleBase.h"
// #include <json/json.hpp>
// #include <fstream>
// #include <iostream>
// #include <sstream>

using namespace std;

class KeyBasedSample : public SampleBase
{
    string license_key ;
public:
    KeyBasedSample( LicenseSpring::LicenseManager::ptr_t manager ) : SampleBase( manager ) { }
    void runOnline( bool deactivateAndRemove = false ) override;
    void runOffline( bool deactivateAndRemove = false ) override;
};

