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
using namespace std;
int main(int argc, char** argv)
{

    bool deactivateAndRemove = false; // deactivate and remove the license after sample execution
    if(argc == 2 ){
        string v1=argv[1];
        int da = stol(v1);
        if( da == 1 || da == 0 )
            deactivateAndRemove = da == 1 ? true:false;
    }

#ifdef _WIN32
    // Enable displaying Unicode symbols in console (custom fields and metadata are UTF-8 encoded)
    SetConsoleOutputCP( CP_UTF8 );
    setvbuf( stdout, nullptr, _IOFBF, 1000 );
#endif

    try
    {
        AppConfig appConfig( "C++ Sample", "3.1" );

        auto pConfiguration = appConfig.createLicenseSpringConfig( );

        std::cout << "------------- General info -------------" << std::endl;
        std::cout << pConfiguration->getAppName() + ' ' << pConfiguration->getAppVersion() << std::endl;
        std::cout << "LicenseSpring SDK version: " << pConfiguration->getSdkVersion() << std::endl;
        std::cout << "LicenseSpring API version: " << pConfiguration->getLicenseSpringAPIVersion() << std::endl;
        std::cout << "Determined OS version:     " << pConfiguration->getOsVersion() << std::endl;
        std::cout << "Hardware ID: " << pConfiguration->getHardwareID() << std::endl;
        std::cout << std::endl;

        std::cout << "------------- Network info -------------" << std::endl;
        std::cout << "Host name:   " << pConfiguration->getNetworkInfo().hostName() << std::endl;
        std::cout << "Local IP:    " << pConfiguration->getNetworkInfo().ip() << std::endl;
        std::cout << "MAC address: " << pConfiguration->getNetworkInfo().mac() << std::endl;
        std::cout << std::endl;


        auto licenseManager = LicenseManager::create( pConfiguration );

        // Get basic information about configured product
        auto productInfo = licenseManager->getProductDetails( true );

        std::cout << "------------- Product info -------------" << std::endl;
        std::cout << "Product name:             " << productInfo.productName() << std::endl;
        std::cout << "Virtual machines allowed: " << productInfo.isVMAllowed() << std::endl;
        std::cout << "Trial allowed:            " << productInfo.isTrialAllowed() << std::endl;
        std::cout << "Metadata:                 " << productInfo.metadata() << std::endl;

        // auto timeout = productInfo.floatingLicenseTimeout();
        // if( timeout > 0 )
        //     std::cout << "Floating license timeout: " << timeout << " min" << std::endl;

        // if( productInfo.isTrialAllowed() && productInfo.trialPeriod() > 0 )
        // {
        //     std::string periodStr = "Trial period:             " +
        //         std::to_string( productInfo.trialPeriod() ) + " day";
        //     if( productInfo.trialPeriod() > 1 )
        //         periodStr += "s";
        //     std::cout << periodStr << std::endl;
        // }

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

