#pragma once

#include <LicenseSpring/LicenseManager.h>

struct ConfigHelper;

class SampleBase
{
public:
    SampleBase( );
    virtual ~SampleBase() {};

    virtual void runOnline( bool deactivateAndRemove = false ) = 0;
    virtual void runOffline( bool deactivateAndRemove = false ) = 0;

    void checkLicenseLocal( LicenseSpring::License::ptr_t license );

    void updateAndCheckLicense( LicenseSpring::License::ptr_t license );
    static void setupAutomaticLicenseUpdates( LicenseSpring::License::ptr_t license );
    static void setupAutomaticFloatingFeatureUpdates( LicenseSpring::License::ptr_t license );

    void cleanUp( LicenseSpring::License::ptr_t license );
    void cleanUpLocal( LicenseSpring::License::ptr_t license );

    void createOfflineActivationRequest( const LicenseSpring::LicenseID& licenseId );
    void updateOfflineLicense( LicenseSpring::License::ptr_t license );

    void printUpdateInfo();
    static void printProductVersionInfo( LicenseSpring::InstallationFile::ptr_t installFile );

    static void PrintLicense( LicenseSpring::License::ptr_t license );

protected:
    LicenseSpring::LicenseManager::ptr_t m_licenseManager;
};

