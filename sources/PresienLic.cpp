
#include "PresienLic.h"
// uncomment to disable assert()
// #define NDEBUG
#include <cassert>
 
 #include <json/json.hpp>
// Use (void) to silence unused warnings.
#define assertm(exp, msg) assert(((void)msg, exp))

//#include "cpu-info.h"
using namespace PRESIEN::BlindSight;

PresienLicense::PresienLicense(REQUEST_CENTRE _req):mRequest(_req){
    Initialize();
}

PresienLicense::PresienLicense(){
    mRequest = REQUEST_CENTRE::INVALID_ACTION;
    Initialize();
}

void PresienLicense::Initialize(){
    mConfig.Initialize();
    m_licenseManager = LicenseManager::create(mConfig.GetBasePtr());
    assertm(m_licenseManager != nullptr, "Failed to Create lmgr."); // assertion fails

    std::cout << "------------- General info -------------" << std::endl;
    std::cout << mConfig.getAppName() + ' ' << mConfig.getAppVersion() << std::endl;
    std::cout << "LicenseSpring SDK version: " << mConfig.getSdkVersion() << std::endl;
    std::cout << "LicenseSpring API version: " << mConfig.getLicenseSpringAPIVersion() << std::endl;
    std::cout << "Determined OS version:     " << mConfig.getOsVersion() << std::endl;
    std::cout << "Hardware ID: " << mConfig.getHardwareID() << std::endl;
    std::cout << std::endl;

    //Update license Data store to the mounted volume
    UpdateDataStorePath();

    ReadProductInfoFromServer();

    ReadTargetPlatformVMInfo();
}

void PresienLicense::ParseCmdArgs(int argc, char**argv){
    
    if(argc != 2 ){
        mRequest = REQUEST_CENTRE::INVALID_ACTION;
        return ;//always offline validate
    }

    string cmd=argv[1];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(),
        [](unsigned char c){ return std::tolower(c); });

    std::string install_cmd = "install";
    std::string update_cmd = "update";
    if (cmd == "install") {
        mRequest = REQUEST_CENTRE::INSTALL;
    }
    else if( cmd == "update"){
        std::cout << "\n Err - Update license action not supported.\n";
        mRequest = REQUEST_CENTRE::UPDATE;
    }
    else if( (cmd == "deactivate")||(cmd == "purge") ){
        std::cout << "\n WARN - deactivation | Purge license action requested.\n";
        mRequest = REQUEST_CENTRE::PURGE;
    }
    else{
        mRequest = REQUEST_CENTRE::VALIDATE;
    }
}

void PresienLicense::UpdateDataStorePath() {
    wstring currPath = m_licenseManager->licenseFilePath();
    wstring newPath = VIRTUAL_BLINDSIGHT_LIC_STORE_PATH + currPath;
    #ifdef __DEBUG
        wcout << "\n Lic filepath = " << m_licenseManager->licenseFilePath() << std::endl;
        wcout << "Lic file name = " << m_licenseManager->licenseFileName() << std::endl;
        wcout << "localdatastorePath = " << localdatastorePath << std::endl;
        wcout << "Lic newPath = " << newPath<< std::endl;
    #endif
    m_licenseManager->setDataLocation(newPath);
}

bool PresienLicense::ProcessRequest(){

    if(mRequest == REQUEST_CENTRE::INVALID_ACTION){
        //AY - if no cmdline args provided, check env variables
        //even no env variable do offline validation.
        mRequest = REQUEST_CENTRE::VALIDATE;
        string req = "VBSINSTALL";
        auto* val = std::getenv(req.c_str());
        if ( val )
        {
            string ans = val;
            if(ans == "1")
            {
                mRequest = REQUEST_CENTRE::INSTALL;
            }
        }
    }
    
    if(mRequest == REQUEST_CENTRE::INVALID_ACTION)
    {
        string req_purge = "VBSPURGE";
        auto* val = std::getenv(req_purge.c_str()); 
        if (val)
        {
            string ans = val;
            if(ans == "1")
            {
                mRequest = REQUEST_CENTRE::PURGE;
            }
            else // if no install and purge then it is validate
                mRequest = REQUEST_CENTRE::VALIDATE;
        }
    }
    switch(mRequest){
            case REQUEST_CENTRE::VALIDATE:
                ValidateLicenseOffline();
                break;
            case REQUEST_CENTRE::INSTALL:
                {
                    if(!ValidateLicenseOffline() )
                    {
                        InstallLicenseOnline();
                    }
                }
                break;
            case REQUEST_CENTRE::UPDATE:
                UpdateLicense();
                break;
            case REQUEST_CENTRE::DEACTIVATE:
            case REQUEST_CENTRE::PURGE:
                DeactivateLicense();
                break;
            default:
                std::cerr << "\n Default action not supported.\n";
                return false;
        }
        return true;
}

void PresienLicense::runOnline(bool dr ){
    auto license = m_licenseManager->getCurrentLicense();
    if (license)
    {
        std::cout << "\nError - License is already installed.";
        // return;
    }

    using json = nlohmann::json;
    std::ifstream f("PresienLic.config.json");
    json data = json::parse(f);
    auto licenseId = LicenseID::fromKey(data["LicKeyValue"]);
    if (licenseId.isEmpty())
    {
        std::cout << "\nError - Invalid License Key supplied.";
        return;
    }

    license = m_licenseManager->activateLicense(licenseId);
    license->addDeviceVariable( "TegraCpuUid", mConfig.GetTegraCpuUid() );
    std::cout << "SUCCESS - License activated successfully.." << std::endl;
    // AY - required to send device variables
    updateAndCheckLicense(license);
#ifdef __DEBUG
    PrintLicense(license);
    printUpdateInfo();
#endif
}

void PresienLicense::runOffline(bool dr ){
    throw("Not yet implemented.");
}

bool PresienLicense::InstallLicenseOnline(){
    std::cout << "\nActivating Install mode -----------";
    std::cout << "\nActivating ------------------------";
    std::cout << "\nActivated Install mode ------------\n";
    
    if( !m_licenseManager->isOnline() )
    {
        std::cout <<"\n Error - Offline system cannot install license.";
        ValidateLicenseOffline();
        return false;
    }

    std::cout <<"\n System is online -----";
    runOnline();
    return true;
}

bool PresienLicense::ValidateLicenseOffline(){
    std::cout <<std::endl<< "Validating offline mode -----------";
    std::cout <<std::endl<< "Validated -------------------------\n";

    auto license = m_licenseManager->getCurrentLicense();
    if(!license){
        std::cerr <<"\n Error - failed to get local license. License not installed.\n";
        return false;
    }

    //Throw exception if failed local check
    checkLicenseLocal( license ); 
    return true;
}

bool PresienLicense::UpdateLicense(){
    std::cout << "\n UpdateLicense -- to be implemented.";
    return false;
}

bool PresienLicense::DeactivateLicense(){
    std::cout << "\n DeactivateLicense and removing from local store -- Online only.";
    if( !m_licenseManager->isOnline() )
    {
        std::cout <<"\n Error - Offline system cannot deactivate license.";
        return false;
    }
    auto license = m_licenseManager->getCurrentLicense();
    if(!license){
        std::cout <<"\nError - No local license found, nothing to remove.";
        return false;
    }
    
    updateAndCheckLicense( license );
    cleanUp( license );
    return true;
}

bool PresienLicense::ReadProductInfoFromServer(){
    auto productInfo = m_licenseManager->getProductDetails(true);
    if (AuthMethodKeyBased != productInfo.authorizationMethod())
    {
        throw("\n Exception - Only KeyBased authentication supported.");
        return false;
    }

#ifdef __DEBUG
        std::cout << "------------- Product info -------------" << std::endl;
        std::cout << "Product name:             " << productInfo.productName() << std::endl;
        std::cout << "Virtual machines allowed: " << productInfo.isVMAllowed() << std::endl;
        std::cout << "Trial allowed:            " << productInfo.isTrialAllowed() << std::endl;
        std::cout << "Metadata:                 " << productInfo.metadata() << std::endl;
#endif
    return true;
}

bool PresienLicense::ReadTargetPlatformVMInfo(){

    // Detect virtualized environment
    if (mConfig.isVMDetectionEnabled())
    {
        std::cout << "Checking for virtual machines..." << std::endl;
        std::string msg;
        if (mConfig.isVM())
        {
            msg = "Virtual machine detected!";
            if (!mConfig.getDetectedVMName().empty())
                msg += " Hypervisor name: " + mConfig.getDetectedVMName();
            
            return true;
        }
        else
            msg = "Check passed, VM not detected.";
        
        std::cout << msg << std::endl<< std::endl;
    }
    return false;
}