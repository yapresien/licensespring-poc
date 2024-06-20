
#include "PresienLic.h"

using namespace PRESIEN::BlindSight;
int main(int argc, char** argv)
{
   
#ifdef _WIN32
    // Enable displaying Unicode symbols in console (custom fields and metadata are UTF-8 encoded)
    SetConsoleOutputCP( CP_UTF8 );
    setvbuf( stdout, nullptr, _IOFBF, 1000 );
#endif
    try
    {
        PresienLicense presienLicense = PresienLicense::GetInstance();
        presienLicense.ParseCmdArgs(argc,argv);
        presienLicense.ProcessRequest();
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

