#pragma once

#include "SampleBase.h"

class UserBasedSample : public SampleBase
{
public:
    UserBasedSample( LicenseSpring::LicenseManager::ptr_t manager ) : SampleBase( manager ) {}
    void runOnline( bool deactivateAndRemove = false ) override;
    void runOffline( bool deactivateAndRemove = false ) override;
};

