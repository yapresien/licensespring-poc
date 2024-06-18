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

#include "cpu-info.h"

#include "MachineId.hpp"
#include "Sha1.hpp"


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
    else if( (cmd == "deactivate")||(cmd == "purge") ){
        std::cout << "\n WARN - deactivation | Purge license action requested.\n";
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
    std::cout << "\n DeactivateLicense and removing from local store -- Online only.";
    if( !licenseManager->isOnline() )
    {
        std::cout <<"\n Error - Offline system cannot deactivate license.";
        return false;
    }
    auto license = licenseManager->getCurrentLicense();
    if(!license){
        std::cout <<"\nError - No local license to remove.";
        auto licenseId = LicenseID::fromKey("HAGJ-ET4H-8CJJ-RKBS" );
        if(licenseId.isEmpty())
        {
            std::cout <<"\nError - Invalid License Key supplied.";
            return false;        
        }
        license = licenseManager->activateLicense( licenseId );
        cout <<"\n Lic filepath = " << licenseManager->licenseFilePath().c_str() << std::endl;
        cout <<"\n Lic file name = " << licenseManager->licenseFileName().c_str() << std::endl;
        wstring localdatastorePath = licenseManager->dataLocation();
        cout <<"\n localdatastorePath = " << localdatastorePath.c_str() << std::endl;

        wstring newPath = L"/PresienLic/" + localdatastorePath;
        
        cout <<"\n Lic newPath = " <<  newPath.c_str() << std::endl;
        licenseManager->setDataLocation(newPath);

    }

    
    shared_ptr<SampleBase> kbsample = nullptr;
    kbsample.reset(new KeyBasedSample(licenseManager));
    kbsample->updateAndCheckLicense( license );
    kbsample->cleanUp( license );
    return true;
}

std::string GetEnv( const std::string & var ) {
     const char * val = std::getenv( var.c_str() );
     if ( val == nullptr ) { // invalid to assign nullptr to std::string
         return "";
     }
     else {
         return val;
     }
}

std::string readFile(std::string const& file) 
{
    std::ifstream is(file);
    if( !is.good() ){
        throw std::runtime_error("Error: tegra stream has errors.");
    }
    std::stringstream ss;
    ss << is.rdbuf();	
	std::string m;
	// Remove ending line character '\n' or '\r\n'.
	std::getline(ss, m);
    return m;
} 
int main(int argc, char** argv)
{
    //cout << getCpuId() << std::endl; 
    auto tegra_cpu_uid = readFile("/sys/module/tegra_fuse/parameters/tegra_chip_uid");

    
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

        int hwid_opt = 2;
        if(hwid_opt == 1){
            //license spring sdk generated hwid
            std::cout << "Hardware ID: " << pConfiguration->getHardwareID() << std::endl;
        }
        else if(hwid_opt==2){
            //shared as environment variable to docker run e.g.
            //sudo docker run -e MAC1=`cat /sys/class/net/eth0/address` -e TARGETHOSTNAME=$HOSTNAME -e CUSTOMER_SSN="HEXAGONSSN1234" --entrypoint /bin/sh -it lsdemo:2.0
            //OR
            //sudo docker run -e MAC1=`ip link show eth0 | grep link/ether | awk '{print $2}'` -e TARGETHOSTNAME=$HOSTNAME -e CUSTOMER_SSN="HEXAGONSSN1234" --entrypoint /bin/sh -it lsdemo:2.0
            //MAC1 - 48B02D55DE70
            //TARGETHOSTNAME - CHEWY-CARAMEL
            //CUSTOMER-SSN - HEXAGONSSN1234
            
            //string - 48B02D55DE70CHEWY-CARAMELHEXAGONSSN1234
            //SHA1 - 2c2af7f48fc97ed259829851769b60b16f7f5341
            string MAC0 = GetEnv("MAC1");
            std::transform(MAC0.begin(), MAC0.end(), MAC0.begin(),
                [](unsigned char c){ return std::toupper(c); });
            string MAC1;
            for(auto& v: MAC0){
                if(v !=':')
                    MAC1+= v;
            }
            std::cout << "MAC ID: " << MAC1 << std::endl;

            string TARGETHOSTNAME = GetEnv("TARGETHOSTNAME");
            std::transform(TARGETHOSTNAME.begin(), TARGETHOSTNAME.end(), TARGETHOSTNAME.begin(),
                [](unsigned char c){ return std::toupper(c); });

            std::cout << "TARGETHOSTNAME: " << TARGETHOSTNAME << std::endl;

            string CUSTOMER_SSN = GetEnv("CUSTOMER_SSN");
            std::transform(CUSTOMER_SSN.begin(), CUSTOMER_SSN.end(), CUSTOMER_SSN.begin(),
                [](unsigned char c){ return std::toupper(c); });

            std::cout << "CUSTOMER_SSN: " << CUSTOMER_SSN << std::endl;

            SHA1 checksum;
            auto sfinal = MAC1+TARGETHOSTNAME+CUSTOMER_SSN;
            // checksum.update(sfinal.c_str());
            // string hw_sha1 = checksum.final();

            checksum.update(tegra_cpu_uid.c_str());
            string hw_sha1 = checksum.final();
            
            std::cout << "Input String: " << tegra_cpu_uid ;
            cout << "\nPresien HardWareID : " <<  hw_sha1 << std::endl;          
            pConfiguration->setHardwareID(hw_sha1);

        }else if(hwid_opt==3){
            //generate own hwid algorithm
            //however inside docker not able to read 
            //1. hostname, 2. mac, 3. cpuid 
            //cout << getCpuId() << std::endl; 
            //cout << machineid::machineHash() << std::endl; 
        }else{

        }

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
                {
                    if(!ValidateLicenseOffline(lmgr, deactivateAndRemove) )
                    {
                        InstallLicenseOnline(lmgr, deactivateAndRemove);
                    }
                }
                break;
            case ACTION_CENTRE::UPDATE:
                UpdateLicense(lmgr);
                break;
            case ACTION_CENTRE::DEACTIVATE:
            case ACTION_CENTRE::PURGE:
                DeactivateLicense(lmgr);
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

