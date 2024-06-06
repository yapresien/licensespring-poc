#include "KeyBasedSample.h"
#include "UserBasedSample.h"
#include "AppConfig.h"

#include <LicenseSpring/Exceptions.h>
#include <LicenseSpring/FloatingClient.h>
#include <iostream>
#include <thread>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

using namespace LicenseSpring;

int TestFloatingServer( LicenseSpring::Configuration::ptr_t config );

int main()
{
#ifdef _WIN32
    // Enable displaying Unicode symbols in console (custom fields and metadata are UTF-8 encoded)
    SetConsoleOutputCP( CP_UTF8 );
    setvbuf( stdout, nullptr, _IOFBF, 1000 );
#endif

    try
    {
        AppConfig appConfig( "C++ Sample", "3.1" );

        auto pConfiguration = appConfig.createLicenseSpringConfig();

        std::cout << "------------- General info -------------" << std::endl;
        std::cout << pConfiguration->getAppName() + ' ' << pConfiguration->getAppVersion() << std::endl;
        std::cout << "LicenseSpring SDK version: " << pConfiguration->getSdkVersion() << std::endl;
        std::cout << "LicenseSpring API version: " << pConfiguration->getLicenseSpringAPIVersion() << std::endl;
        std::cout << "Determined OS version:     " << pConfiguration->getOsVersion() << std::endl;
        std::cout << std::endl;

        std::cout << "------------- Network info -------------" << std::endl;
        std::cout << "Host name:   " << pConfiguration->getNetworkInfo().hostName() << std::endl;
        std::cout << "Local IP:    " << pConfiguration->getNetworkInfo().ip() << std::endl;
        std::cout << "MAC address: " << pConfiguration->getNetworkInfo().mac() << std::endl;
        std::cout << std::endl;

        //return TestFloatingServer( pConfiguration ); // Uncomment this line to test Floating server

        bool deactivateAndRemove = false; // deactivate and remove the license after sample execution

        auto licenseManager = LicenseManager::create( pConfiguration );

        // Get basic information about configured product
        auto productInfo = licenseManager->getProductDetails( true );

        std::cout << "------------- Product info -------------" << std::endl;
        std::cout << "Product name:             " << productInfo.productName() << std::endl;
        std::cout << "Virtual machines allowed: " << productInfo.isVMAllowed() << std::endl;
        std::cout << "Trial allowed:            " << productInfo.isTrialAllowed() << std::endl;
        std::cout << "Metadata:                 " << productInfo.metadata() << std::endl;

        auto timeout = productInfo.floatingLicenseTimeout();
        if( timeout > 0 )
            std::cout << "Floating license timeout: " << timeout << " min" << std::endl;

        if( productInfo.isTrialAllowed() && productInfo.trialPeriod() > 0 )
        {
            std::string periodStr = "Trial period:             " +
                std::to_string( productInfo.trialPeriod() ) + " day";
            if( productInfo.trialPeriod() > 1 )
                periodStr += "s";
            std::cout << periodStr << std::endl;
        }

        std::shared_ptr<SampleBase> sample = nullptr;
        std::string authMethod = "Authorization method:     ";

        if( productInfo.authorizationMethod() == AuthMethodKeyBased )
        {
            sample.reset( new KeyBasedSample( licenseManager ) );
            authMethod += "Key-based";
        }
        else
        {
            sample.reset( new UserBasedSample( licenseManager ) );
            authMethod += "User-based";
        }

        std::cout << authMethod << std::endl << std::endl;

        // Detect virtualized environment
        if( pConfiguration->isVMDetectionEnabled() )
        {
            std::cout << "Checking for virtual machines..." << std::endl;
            std::string msg;
            if( pConfiguration->isVM() )
            {
                msg = "Virtual machine detected!";
                if( !pConfiguration->getDetectedVMName().empty() )
                    msg += " Hypervisor name: " + pConfiguration->getDetectedVMName();
            }
            else
                msg = "Check passed, VM not detected.";
            std::cout << msg << std::endl << std::endl;
        }

        // Print Product latest version if available
        auto productInstallPackage = productInfo.installationFile();
        if( productInstallPackage )
        {
            std::cout << "Latest installation package information" << std::endl;
            SampleBase::printProductVersionInfo( productInstallPackage );
        }

        // Run appropriate sample
        if( licenseManager->isOnline() )
            sample->runOnline( deactivateAndRemove );
        else
            sample->runOffline( deactivateAndRemove );

        return 0;
    }
    catch( const LicenseSpringException& ex )
    {
        std::cout << "LicenseSpring exception encountered: " << ex.what();
        return static_cast<int>( ex.getCode() );
    }
    catch( const std::exception& ex )
    {
        std::cout << "Standard exception encountered: " << ex.what();
        return -1;
    }
    catch( ... )
    {
        std::cout << "Unknown exception encountered!";
        return -3;
    }
}

int TestFloatingServer( LicenseSpring::Configuration::ptr_t config )
{
    // Optionaly you can provide generic user info to config, it will be passed to the FloatingServer
    config->setUserInfo( "Test user info" );

    auto fc = FloatingClient::create( config );

    std::cout << "Checking connection to Floating server...\n";
    if( fc->isOnline( true ) )
        std::cout << "Connection established\n\n";

    auto serverInfo = fc->getServerInfo();
    const auto& serverAddresses = serverInfo->servers();
    if( !serverAddresses.empty() || serverInfo->registrationExpiry() != 0 )
    {
        std::cout << "------------- Server info -------------\n";
        std::cout << "Client registration expires in " << serverInfo->registrationExpiry() << " minutes\n";
        std::cout << "Servers:\n";
        for( const auto& serverIP : serverAddresses )
            std::cout << serverIP;
        std::cout << "\n\n";
    }

    std::cout << "Registering floating license...\n";
    std::string clientId; // can be anything, e.g. identifier of this user, machine id or app instance id
    auto license = fc->getCurrentLicense();
    if( license == nullptr )
    {
        clientId = config->getHostName(); // for example let it be host name
        license = fc->registerLicense( clientId );
    }
    else
    {
        clientId = license->floatingClientId();
        license->registerFloatingLicense();
    }
    std::cout << "Floating license successfully registered to client " << clientId << "\n";

    // Increase license consumption
    if( license->type() == LicenseTypeConsumption )
    {
        std::cout << "Increasing and updating consumption...\n";
        license->updateConsumption( 1 );
        license->syncConsumption();
        std::cout << "Operation completed successfully\n\n";
    }

    // Increase consumption features
    for( const auto& feature : license->features() )
    {
        if( feature.featureType() == FeatureTypeConsumption )
            license->updateFeatureConsumption( feature.code(), 1 );
    }
    license->syncFeatureConsumption();

    // In order to keep floating license alive automatically in the background thread uncomment the line below
    //SampleBase::setupAutomaticLicenseUpdates( license );

    // Test license borrowing
    //license->borrow(); // or fc->borrowLicense( clientId );

    SampleBase::PrintLicense( license );

    std::cout << "\nUnregistering client...\n";
    // Floating license will be automatically revoked (released) when destroyed,
    // our you can do it manually by the following code
    license->setAutoRelease( false );
    license->releaseFloatingLicense( true );
    // or call
    //fc->unregisterLicense( clientId );
    std::cout << "The client " << clientId << " unregistered successfully\n\n";

    return 0;
}
