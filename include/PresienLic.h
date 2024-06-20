
#include <iostream>
#include <LicenseSpring/Exceptions.h>

#include "SampleBase.h"

#include <iostream>
#include <thread>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <algorithm>
#include <cctype>
#include <string>

#include "AppConfig.h"
#include "Sha1.hpp"

using namespace std;
using namespace LicenseSpring;

namespace PRESIEN::BlindSight{

    using SpringConfigPtr = LicenseSpring::Configuration::ptr_t;
    enum class REQUEST_CENTRE{
        INVALID_ACTION,
        VALIDATE = 0,
        INSTALL,
        UPDATE,
        DEACTIVATE,
        PURGE
    };

    class PresienLicenseConfig :public LicenseSpring::Configuration{

        LicenseSpring::Configuration::ptr_t _pConfig;

        std::string _readFile(std::string const& file){
            std::ifstream is(file);
            if (!is.good())
            {
                throw std::runtime_error("Error: tegra stream has errors.");
            }
            std::stringstream ss;
            ss << is.rdbuf();
            std::string m;
            // Remove ending line character '\n' or '\r\n'.
            std::getline(ss, m);
            return m;
        }

        std::string _getEnv(const std::string &var)
        {
            const char *val = std::getenv(var.c_str());
            if (val == nullptr)
            { // invalid to assign nullptr to std::string
                return "";
            }
            else
            {
                return val;
            }
        }

        string mTegraCpuUid;

        bool _updateToPresienHardwareID()
        {
            
            try
            {
                mTegraCpuUid = _readFile("/sys/module/tegra_fuse/parameters/tegra_chip_uid");
            }
            catch (...)
            {
#ifdef __DEBUG
                tegra_cpu_uid = readFile("/etc/machine-id");
                cout << "\n Tegra stream not found, generalizing to x86 for debugging purpose only.";
#else
                return 0;
#endif
            }
            SHA1 checksum;
            // generate own hwid algorithm
            checksum.update(mTegraCpuUid.c_str());
            string hw_sha1 = checksum.final();

            std::cout << "Input String: " << mTegraCpuUid << std::endl;
            cout << "Presien HardWareID : " << hw_sha1 << std::endl;
            _pConfig->setHardwareID(hw_sha1);

            return true;
        }

        bool _updateToPresienHardwareIdUsingEnv()
        {
            //shared as environment variable to docker run e.g.
            //sudo docker run -e MAC1=`cat /sys/class/net/eth0/address` -e TARGETHOSTNAME=$HOSTNAME -e CUSTOMER_SSN="HEXAGONSSN1234" --entrypoint /bin/sh -it lsdemo:2.0
            //OR
            //sudo docker run -e MAC1=`ip link show eth0 | grep link/ether | awk '{print $2}'` -e TARGETHOSTNAME=$HOSTNAME -e CUSTOMER_SSN="HEXAGONSSN1234" --entrypoint /bin/sh -it lsdemo:2.0
            //MAC1 - 48B02D55DE70
            //TARGETHOSTNAME - CHEWY-CARAMEL
            //CUSTOMER-SSN - HEXAGONSSN1234
            
            //string - 48B02D55DE70CHEWY-CARAMELHEXAGONSSN1234
            //SHA1 - 2c2af7f48fc97ed259829851769b60b16f7f5341
            string MAC0 = _getEnv("MAC1");
            std::transform(MAC0.begin(), MAC0.end(), MAC0.begin(),
                [](unsigned char c){ return std::toupper(c); });
            string MAC1;
            for(auto& v: MAC0){
                if(v !=':')
                    MAC1+= v;
            }
            std::cout << "MAC ID: " << MAC1 << std::endl;

            string TARGETHOSTNAME = _getEnv("TARGETHOSTNAME");
            std::transform(TARGETHOSTNAME.begin(), TARGETHOSTNAME.end(), TARGETHOSTNAME.begin(),
                [](unsigned char c){ return std::toupper(c); });

            std::cout << "TARGETHOSTNAME: " << TARGETHOSTNAME << std::endl;

            string CUSTOMER_SSN = _getEnv("CUSTOMER_SSN");
            std::transform(CUSTOMER_SSN.begin(), CUSTOMER_SSN.end(), CUSTOMER_SSN.begin(),
                [](unsigned char c){ return std::toupper(c); });

            std::cout << "CUSTOMER_SSN: " << CUSTOMER_SSN << std::endl;           
            auto sfinal = MAC1+TARGETHOSTNAME+CUSTOMER_SSN;
            SHA1 checksum;
            checksum.update(sfinal.c_str());
            string hw_sha1 = checksum.final();
            
            std::cout << "Input String: " << sfinal << std::endl;
            cout << "ENV HardWareID : " <<  hw_sha1 << std::endl;          
            _pConfig->setHardwareID(hw_sha1);
            return true;
        }

    public:
        PresienLicenseConfig() = default;
        virtual ~PresienLicenseConfig() = default;
        PresienLicenseConfig(const PresienLicenseConfig &) = default;
        PresienLicenseConfig &operator=(const PresienLicenseConfig &) = default;
        PresienLicenseConfig(PresienLicenseConfig &&) = default;
        PresienLicenseConfig &operator=(PresienLicenseConfig &&) = default;

        void Initialize()
        {
            AppConfig appConfig("C++ Sample", "3.1");
            _pConfig = appConfig.createLicenseSpringConfig();

#ifdef __DEBUG
            std::cout << "------------- Network info -------------" << std::endl;
            std::cout << "Host name:   " << _pConfig->getNetworkInfo().hostName() << std::endl;
            std::cout << "Local IP:    " << _pConfig->getNetworkInfo().ip() << std::endl;
            std::cout << "MAC address: " << _pConfig->getNetworkInfo().mac() << std::endl;
            std::cout << std::endl;
#endif
            _updateToPresienHardwareID();
        }

        const std::string& GetTegraCpuUid()const { return mTegraCpuUid;}
        SpringConfigPtr GetBasePtr() const
        {
            return _pConfig;
        }
    };

    class PresienLicense : public SampleBase{
        
        PresienLicenseConfig mConfig;    
        REQUEST_CENTRE mRequest;
        const wstring VIRTUAL_BLINDSIGHT_LIC_STORE_PATH =L"/PresienVBS";

        private:
            PresienLicense();
            PresienLicense(REQUEST_CENTRE req);
            void Initialize();
            bool InstallLicenseOnline();
            bool ValidateLicenseOffline();
            bool UpdateLicense();
            bool DeactivateLicense();
            void UpdateDataStorePath();
            bool ReadProductInfoFromServer();
            bool ReadTargetPlatformVMInfo();

        public:
            static PresienLicense& GetInstance(){
                static PresienLicense presienLicense;
                return presienLicense;
            }
            void ParseCmdArgs(int argc, char**argv);
            bool ProcessRequest();
            
            virtual void runOnline( bool deactivateAndRemove = false ) override;
            virtual void runOffline( bool deactivateAndRemove = false ) override;

    };
};