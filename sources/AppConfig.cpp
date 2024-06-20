#include "AppConfig.h"
#include <LicenseSpring/EncryptString.h>

LicenseSpring::Configuration::ptr_t AppConfig::createLicenseSpringConfig() const
{
    // Optionally you can provide full path where license file will be stored, hardwareID and other options
    LicenseSpring::ExtendedOptions options;
    options.collectNetworkInfo( true );
    options.enableLogging( true );
    options.enableVMDetection( true );

    // Provide your LicenseSpring credentials here, please keep them safe
    return LicenseSpring::Configuration::Create(
        EncryptStr( "da262440-9ad3-47f4-b5d5-3612c0f08622" ), // your LicenseSpring API key (UUID)
        EncryptStr( "1h1ORaBjA6JZJbB3gJenU3-dz5nkcwS4v_tm6hGmWZU" ), // your LicenseSpring Shared key
        EncryptStr( "BS110" ), // product code that you specified in LicenseSpring for your application
        appName, appVersion, options );
}
