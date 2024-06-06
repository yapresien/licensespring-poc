#pragma once

#include "SampleBase.h"

class KeyBasedSample : public SampleBase
{
public:
    KeyBasedSample( LicenseSpring::LicenseManager::ptr_t manager ) : SampleBase( manager ) {}
    void runOnline( bool deactivateAndRemove = false ) override;
    void runOffline( bool deactivateAndRemove = false ) override;
};

