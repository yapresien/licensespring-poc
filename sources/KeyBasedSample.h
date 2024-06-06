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
    KeyBasedSample( LicenseSpring::LicenseManager::ptr_t manager ) : SampleBase( manager ) {

        // ifstream fJson("Presien.config.json");
        // stringstream buffer;
        // buffer << fJson.rdbuf();
        // auto json = nlohmann::json::parse(buffer.str());    
        // auto uuid_api_key_from_ls = json["ls_presien_uuid_api_key"];
        // auto shared_key_from_ls = json["ls_presien_shared_key"];
        // auto presien_prod_code = json["product_code"];
        // license_key = json["assigned_license_key"];
    }
    void runOnline( bool deactivateAndRemove = false ) override;
    void runOffline( bool deactivateAndRemove = false ) override;
};

