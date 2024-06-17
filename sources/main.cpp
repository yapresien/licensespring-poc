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

#include <algorithm>
#include <cctype>
#include <string>

//#include "cpu-info.h"

#include "MachineId.hpp"


using namespace std;
enum class ACTION_CENTRE{
    INVALID_ACTION,
    VALIDATE = 0,
    INSTALL,
    UPDATE,
    DEACTIVATE,
    PURGE
};

static ACTION_CENTRE gAction = ACTION_CENTRE::INVALID_ACTION;

void PraseCmdParamsIfInstall( int argc, char**argv ){
    
    if(argc != 2 ){
        gAction = ACTION_CENTRE::VALIDATE;
        return ;//always offline validate
    }

    string cmd=argv[1];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(),
        [](unsigned char c){ return std::tolower(c); });

    std::string install_cmd = "install";
    std::string update_cmd = "update";
    if (cmd == "install") {
        gAction = ACTION_CENTRE::INSTALL;
    }
    else if( cmd == "update"){
        std::cout << "\n Err - Update license action not supported.\n";
        gAction = ACTION_CENTRE::UPDATE;
    }
    else if( cmd == "deactivate"){
        std::cout << "\n Err - Deactivate license action not supported.\n";
        gAction = ACTION_CENTRE::DEACTIVATE;
    }else if( cmd == "purge"){
        std::cout << "\n Err - Purge license action not supported.\n";
        gAction = ACTION_CENTRE::PURGE;
    }
    else{
        gAction = ACTION_CENTRE::VALIDATE;
    }
}


bool InstallLicenseOnline( const LicenseManager::ptr_t& licenseManager, 
                            bool deactivateAndRemove){

    std::cout << "\nActivating Install mode -----------";
    std::cout << "\nActivating ------------------------";
    std::cout << "\nActivated Install mode ------------\n";
    
    shared_ptr<SampleBase> kbsample = nullptr;
    kbsample.reset(new KeyBasedSample(licenseManager));
    std::cout << "Authorization method:     Key-based" << std::endl;

    // Print Product latest version if available
    // auto productInstallPackage = productInfo.installationFile();
    // if( productInstallPackage )
    // {
    //     std::cout << "Latest installation package information" << std::endl;
    //     SampleBase::printProductVersionInfo( productInstallPackage );
    // }

    if( !licenseManager->isOnline() )
    {
        std::cout <<"\n Error - Offline system cannot install license.";
        return false;
    }
    
    std::cout <<"\n System is online -----";
    kbsample->runOnline( deactivateAndRemove );
    return true;
}

bool ValidateLicenseOffline( const LicenseManager::ptr_t& licenseManager, 
                            bool deactivateAndRemove=false){
    std::cout << "\n Validating offline mode -----------";
    std::cout << "\nActivating -------------------------";
    std::cout << "\nActivated  -------------------------\n";

    auto license = licenseManager->getCurrentLicense();
    if(!license){
        std::cerr <<"\n Error - failed to get local license. License not installed.\n";
        return false;
    }

    shared_ptr<SampleBase> kbsample = nullptr;
    kbsample.reset(new KeyBasedSample(licenseManager));
    
    //Throw exception if failed local check
    kbsample->checkLicenseLocal( license ); 

    return true;
}

bool UpdateLicense(const LicenseManager::ptr_t& licenseManager){
    std::cout << "\n UpdateLicense -- to be implemented.";
    return false;
}

bool DeactivateLicense(const LicenseManager::ptr_t& licenseManager){
    std::cout << "\n DeactivateLicense -- to be implemented.";
    return false;
}

bool PurgeLicense(const LicenseManager::ptr_t& licenseManager){
    std::cout << "\n PurgeLicense -- to be implemented.";
    return false;
}



// int getCpuId(){
// 	if (!cpuinfo_initialize()) {
// 		fprintf(stderr, "failed to initialize CPU information\n");
// 		exit(EXIT_FAILURE);
// 	}
// #ifdef __ANDROID__
// 	printf("SoC name: %s\n", cpuinfo_get_package(0)->name);
// #else
// 	printf("Packages:\n");
// 	for (uint32_t i = 0; i < cpuinfo_get_packages_count(); i++) {
// 		printf("\t%" PRIu32 ": %s\n", i, cpuinfo_get_package(i)->name);
// 	}
// #endif
// 	printf("Microarchitectures:\n");
// 	for (uint32_t i = 0; i < cpuinfo_get_uarchs_count(); i++) {
// 		const struct cpuinfo_uarch_info* uarch_info = cpuinfo_get_uarch(i);
// 		const char* uarch_string = uarch_to_string(uarch_info->uarch);
// 		if (uarch_string == NULL) {
// 			printf("\t%" PRIu32 "x Unknown (0x%08" PRIx32 "\n",
// 			       uarch_info->core_count,
// 			       (uint32_t)uarch_info->uarch);
// 		} else {
// 			printf("\t%" PRIu32 "x %s\n", uarch_info->core_count, uarch_string);
// 		}
// 	}
// 	printf("Cores:\n");
// 	for (uint32_t i = 0; i < cpuinfo_get_cores_count(); i++) {
// 		const struct cpuinfo_core* core = cpuinfo_get_core(i);
// 		if (core->processor_count == 1) {
// 			printf("\t%" PRIu32 ": 1 processor (%" PRIu32 ")", i, core->processor_start);
// 		} else {
// 			printf("\t%" PRIu32 ": %" PRIu32 " processors (%" PRIu32 "-%" PRIu32 ")",
// 			       i,
// 			       core->processor_count,
// 			       core->processor_start,
// 			       core->processor_start + core->processor_count - 1);
// 		}
// 		const char* vendor_string = vendor_to_string(core->vendor);
// 		const char* uarch_string = uarch_to_string(core->uarch);
// 		if (vendor_string == NULL) {
// 			printf(", vendor 0x%08" PRIx32 " uarch 0x%08" PRIx32 "\n",
// 			       (uint32_t)core->vendor,
// 			       (uint32_t)core->uarch);
// 		} else if (uarch_string == NULL) {
// 			printf(", %s uarch 0x%08" PRIx32 "\n", vendor_string, (uint32_t)core->uarch);
// 		} else {
// 			printf(", %s %s\n", vendor_string, uarch_string);
// 		}
// 	}
// 	printf("Logical processors");
// #if defined(__linux__)
// 	printf(" (System ID)");
// #endif
// 	printf(":\n");
// 	for (uint32_t i = 0; i < cpuinfo_get_processors_count(); i++) {
// 		const struct cpuinfo_processor* processor = cpuinfo_get_processor(i);
// 		printf("\t%" PRIu32 "", i);

// #if defined(__linux__)
// 		printf(" (%" PRId32 ")", processor->linux_id);
// #endif

// #if CPUINFO_ARCH_X86 || CPUINFO_ARCH_X86_64
// 		printf(": APIC ID 0x%08" PRIx32 "\n", processor->apic_id);
// #else
// 		printf("\n");
// #endif
// 	}

//     return cpuinfo_get_processors_count();
// }

int main(int argc, char** argv)
{
    cout << getCpuId() << std::endl; 
    cout << machineid::machineHash() << std::endl; 

    return 0;
#ifdef _WIN32
    // Enable displaying Unicode symbols in console (custom fields and metadata are UTF-8 encoded)
    SetConsoleOutputCP( CP_UTF8 );
    setvbuf( stdout, nullptr, _IOFBF, 1000 );
#endif

    bool deactivateAndRemove = false; // deactivate and remove the license after sample execution
    try
    {
        PraseCmdParamsIfInstall(argc, argv);
        AppConfig appConfig("C++ Sample", "3.1");

        auto pConfiguration = appConfig.createLicenseSpringConfig();

        std::cout << "------------- General info -------------" << std::endl;
        std::cout << pConfiguration->getAppName() + ' ' << pConfiguration->getAppVersion() << std::endl;
        std::cout << "LicenseSpring SDK version: " << pConfiguration->getSdkVersion() << std::endl;
        std::cout << "LicenseSpring API version: " << pConfiguration->getLicenseSpringAPIVersion() << std::endl;
        std::cout << "Determined OS version:     " << pConfiguration->getOsVersion() << std::endl;
        std::cout << "Hardware ID: " << pConfiguration->getHardwareID() << std::endl;
        std::cout << std::endl;
        pConfiguration->setHardwareID("HEXAGONSN1102323");

        std::cout << "------------- Network info -------------" << std::endl;
        std::cout << "Host name:   " << pConfiguration->getNetworkInfo().hostName() << std::endl;
        std::cout << "Local IP:    " << pConfiguration->getNetworkInfo().ip() << std::endl;
        std::cout << "MAC address: " << pConfiguration->getNetworkInfo().mac() << std::endl;
        std::cout << std::endl;

        auto lmgr = LicenseManager::create(pConfiguration);
        // Get basic information about configured product - only possible in online mode
        auto productInfo = lmgr->getProductDetails(true);

        std::cout << "------------- Product info -------------" << std::endl;
        std::cout << "Product name:             " << productInfo.productName() << std::endl;
        std::cout << "Virtual machines allowed: " << productInfo.isVMAllowed() << std::endl;
        std::cout << "Trial allowed:            " << productInfo.isTrialAllowed() << std::endl;
        std::cout << "Metadata:                 " << productInfo.metadata() << std::endl;

        if (AuthMethodKeyBased != productInfo.authorizationMethod())
        {
            throw("\n Exception - Only KeyBased authentication supported.");
            return false;
        }
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

        switch(gAction){
            case ACTION_CENTRE::VALIDATE:
                ValidateLicenseOffline(lmgr, deactivateAndRemove);
                break;
            case ACTION_CENTRE::INSTALL:
                InstallLicenseOnline(lmgr, deactivateAndRemove);
                break;
            case ACTION_CENTRE::UPDATE:
                UpdateLicense(lmgr);
                break;
            case ACTION_CENTRE::DEACTIVATE:
                DeactivateLicense(lmgr);
                break;
            case ACTION_CENTRE::PURGE:
                PurgeLicense(lmgr);
                break;
            default:
                std::cerr << "\n Default action not supported.";
        }


        std::cout <<"\n\n";
        return 0;
    }
    catch( const LicenseSpringException& ex )
    {
        std::cout << "LicenseSpring exception encountered: " << ex.what();std::cout <<"\n\n";
        return static_cast<int>( ex.getCode() );
    }
    catch( const std::exception& ex )
    {
        std::cout << "Standard exception encountered: " << ex.what();std::cout <<"\n\n";
        return -1;
    }
    catch( ... )
    {
        std::cout << "Unknown exception encountered!";std::cout <<"\n\n";
        return -3;
    }
}

