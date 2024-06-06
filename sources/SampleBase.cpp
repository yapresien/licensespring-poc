#include "SampleBase.h"
#include <iostream>
#include <thread>

using namespace LicenseSpring;

std::string TmToStr( const tm& dateTime )
{
    return TmToString( dateTime, "%d-%m-%Y %H:%M:%S" );
};

SampleBase::SampleBase( LicenseManager::ptr_t manager )
    : m_licenseManager( manager )
{
}

void SampleBase::checkLicenseLocal( License::ptr_t license )
{
    std::cout << "License successfully loaded, performing local check of the license..." << std::endl;
    // it's highly recommended to perform a localCheck (offline check) on every startup
    // to be ensure that license file wasn't copied from another computer and license in a valid state
    try
    {
        license->localCheck(); // throws exceptions in case of errors, see documentation
    }
    catch( const DeviceNotLicensedException& ex )
    {
        // Below is an example on how to upgrade to new or other device id algorithm
        std::cout << "Local check failed: " << ex.what() << std::endl;
        std::cout << "Trying to upgrade to newer device id algorithm..." << std::endl;
        license = m_licenseManager->relinkLicense( WinCryptographyId );
        std::cout << "License successfully linked to new device id." << std::endl;
        return;
    }
    catch( const FloatingTimeoutExpiredException& ex )
    {
        std::cout << ex.what() << std::endl;
        auto endDate = TmToStr( license->floatingEndDateTime() );
        std::cout << "Registration of this floating license has expired at: " << endDate << std::endl;
        std::cout << "Trying to register floating license..." << std::endl;
        license->registerFloatingLicense(); // this call is equivalent to online license check
        std::cout << "License successfully checked in" << std::endl;
        return;
    }
    std::cout << "Local validation successful" << std::endl;
}

void SampleBase::updateAndCheckLicense( License::ptr_t license )
{
    // Increase consumption license
    if( license->type() == LicenseTypeConsumption )
    {
        license->updateConsumption( 1 );
        license->syncConsumption(); // this call is not necessary, consumption will be synced during online check
    }

    // Increase consumption features
    for( const auto& feature : license->features() )
    {
        if( feature.featureType() == FeatureTypeConsumption )
            license->updateFeatureConsumption( feature.code(), 1 );
    }
    license->syncFeatureConsumption(); // this call is not necessary, features will be synced during online check

    // Register floating features
    for( const auto& feature : license->features() )
    {
        if( !(feature.isFloating() || feature.isOfflineFloating()) )
            continue;

        std::cout << "Registering floating feature " << feature.code() << std::endl;
        license->registerFloatingFeature( feature.code() );
        // need to reload feature
        auto updatedFeature = license->feature( feature.code() );
        std::cout << updatedFeature.toString() << std::endl;

        std::cout << "Releasing floating feature " << feature.code() << std::endl;
        license->releaseFloatingFeature( feature.code() );
        updatedFeature = license->feature( feature.code() );
        std::cout << updatedFeature.toString() << std::endl;
    }

    // Sync license with the platform
    std::cout << "Checking license online..." << std::endl;
    bool includeExpiredFeatures = false;
    license->check( InstallFileFilter(), includeExpiredFeatures ); // throws exceptions in case of errors
    std::cout << "License successfully checked" << std::endl;
    if( license->isGracePeriodStarted() )
    {
        std::cout << "Grace period started!" << std::endl;
        std::cout << license->gracePeriodHoursRemaining() << " hours till the end of grace period." << std::endl;
        std::cout << "Grace period end date time: " << TmToStr( license->gracePeriodEndDateTime() ) << std::endl;
    }

    // Example of sending and getting custom data to the LS backend (see device variables on the platform)
    std::cout << "Sending custom data to the LicenseSpring..." << std::endl;
    auto approxCoreCount = std::thread::hardware_concurrency();
    license->addDeviceVariable( "CPU_Cores", std::to_string( approxCoreCount ) );
    license->sendDeviceVariables();
    // bool argument below means request variables list from the backend
    auto deviceVariables = license->getDeviceVariables( true );
    for( const auto& variable : deviceVariables )
    {
        std::cout << "Device variable: " << variable.name() << ", value: "
            << variable.value() << ", last time updated: " << TmToStr( variable.dateTimeUpdated() ) << std::endl;
    }
    std::cout << "Operation completed successfully" << std::endl;
}

void SampleBase::setupAutomaticLicenseUpdates( License::ptr_t license )
{
    // You can borrow floating license for some period of time or till some date time
    // During borrowing period there is no need to check-in license by floating timeout
    //license->borrow();
    //return;

    // Setup a watchdog (background thread), it will automatically check license online
    // This especially useful for floating license, because it's required to keep it alive by doing periodic online checks.
    // The watchdog will call a provided callback in case of errors.

    std::weak_ptr<License> wpLicense( license );
    license->setupLicenseWatchdog( [wpLicense]( const LicenseSpringException& ex )
        {
            // Attention, do not capture License::ptr_t (shared_ptr), this will lead to problems.

            std::cout << std::endl << "License check failed: " << ex.what() << std::endl;

            if( ex.getCode() == eMaxFloatingReached )
            {
                std::cout << "Application cannot use this license at the moment because floating license limit reached." << std::endl;
                exit( 0 );
            }

            // Ignore other errors and continue running watchdog if possible
            if( auto pLicense = wpLicense.lock() )
            {
                if( pLicense->isValid() )
                    pLicense->resumeLicenseWatchdog();
            }
        } );

    // Let background thread update the license before printing
    std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
}

void SampleBase::setupAutomaticFloatingFeatureUpdates( License::ptr_t license )
{
    // Set up a watchdog (background thread), it will automatically check all registered floating license features
    // The watchdog will call a provided callback in case of errors.

    std::weak_ptr<License> wpLicense( license );
    license->setupFeatureWatchdog( [wpLicense]( const LicenseSpringException& ex )
                                   {
                                       // Attention, do not capture License::ptr_t (shared_ptr), this will lead to problems.

                                       std::cout << std::endl << "License check failed: " << ex.what() << std::endl;

                                       // Ignore other errors and continue running watchdog if possible
                                       if( auto pLicense = wpLicense.lock() )
                                       {
                                           if( pLicense->isValid() )
                                               pLicense->resumeFeatureWatchdog();
                                       }
                                   } );

    // set up your own floating features here
    std::vector<std::string> featureCodes { "floating-feature-1", "floating-feature-2" };

    try
    {
        for (const std::string &featureCode: featureCodes)
            license->registerFloatingFeature(featureCode);
    }
    catch ( const LicenseSpring::MaxFloatingReachedException& e )
    {
        std::cout << "Error while checking floating feature: max floating reached!" << std::endl;
    }
    catch( const LicenseSpring::LicenseSpringException& e )
    {
        std::cout << "Error while checking floating feature: " << ' ' << e.getCode() << ' ' << e.what() << std::endl;
    }

    // sleep for 100 seconds and check features in the meantime
    std::this_thread::sleep_for( std::chrono::seconds( 100) );
    try
    {
        // released features are automatically removed from the watchdog
        for( const std::string& featureCode : featureCodes )
            license->releaseFloatingFeature( featureCode );
    }
    catch( const LicenseSpring::LicenseSpringException& e )
    {
        std::cout << "Error while releasing floating feature: " << ' ' << e.getCode() << ' ' << e.what() << std::endl;
    }

    license->stopFeatureWatchdog();
}

void SampleBase::cleanUp( License::ptr_t license )
{
    if( license->deactivate( true ) )
        std::cout << "License deactivated successfully." << std::endl << std::endl;
}

void SampleBase::cleanUpLocal( License::ptr_t license )
{
    auto filePath = license->deactivateOffline();
    m_licenseManager->clearLocalStorage();
    std::cout << "To finish deactivation process please upload deactivation request file to the LicenseSpring portal." << std::endl;
    std::cout << "File path: " << filePath.c_str() << std::endl << std::endl;
}

void SampleBase::createOfflineActivationRequest( const LicenseID& licenseId )
{
    std::cout << "Creating offline activation request file..." << std::endl;
    auto filePath = m_licenseManager->createOfflineActivationFile( licenseId );
    std::cout << "File created: " << std::endl;
    std::wcout << filePath << std::endl;
    std::cout << "Please upload that request file to LicenseSpring offline activation portal to get response file.\n";
    std::cout << "Offline activation portal address: https://offline.licensespring.com" << std::endl;
}

void SampleBase::updateOfflineLicense( LicenseSpring::License::ptr_t license )
{
    // Here is an example how to update offline license.
    // You can download refresh file from LicenseSpring platform, see Devices section of particular license.
    // Choose active device for which you would like to update the license and press "Download license refresh file" button.
    // You may also create refresh file using Management API.

    // Assign license refresh file path below
    std::wstring refreshFilePath = L"license_refresh.lic";
    if( license->updateOffline( refreshFilePath ) )
        std::cout << "\nLicense refresh file successfully applied\n";
}

void SampleBase::printUpdateInfo()
{
    if( !m_licenseManager )
        return;

    auto license = m_licenseManager->getCurrentLicense();
    if( license == nullptr )
        return;

    auto versionList = m_licenseManager->getVersionList( license->id() );
    if( versionList.empty() )
        return;

    std::cout << std::endl << "------------- Update info -------------" << std::endl;
    std::cout << "Total app versions available: " << versionList.size() << std::endl;

    auto installFile = m_licenseManager->getInstallationFile( license->id(), versionList.at( versionList.size() - 1 ) );
    if( installFile )
    {
        std::cout << "Latest installation package information" << std::endl;
        printProductVersionInfo( installFile );
    }
}

void SampleBase::printProductVersionInfo( LicenseSpring::InstallationFile::ptr_t installFile )
{
    if( installFile == nullptr )
        return;
    std::cout << "Product version: " << installFile->version() << std::endl;
    std::cout << "Release date: " << installFile->releaseDate() << std::endl;
    std::cout << "Required version for update: " << installFile->requiredVersion() << std::endl;
    std::cout << "URL for downloading: " << installFile->url() << std::endl;
    std::cout << "Md5 hash: " << installFile->md5Hash() << std::endl;
    std::cout << "Environment: " << installFile->environment() << std::endl;
    std::cout << "Eula Link: " << installFile->eulaLink() << std::endl;
    std::cout << "Release Notes Link: " << installFile->releaseNotesLink() << std::endl;
    std::cout << "Size: " << installFile->size() << std::endl;
    std::cout << "Channel: " << installFile->channel() << std::endl;
    std::cout << std::endl;
}

void SampleBase::PrintLicense( License::ptr_t license )
{
    if( license == nullptr )
        return;

    std::cout << std::endl << "------------- License info -------------" << std::endl;

    auto formatStr = []( std::string& str, const std::string& value )
    {
        if( value.empty() )
            return;
        if( !str.empty() )
            str.append( " " );
        str.append( value );
    };

    const auto& licenseOwner = license->owner();
    {
        std::string ownerInfo; // license owner info string
        formatStr( ownerInfo, licenseOwner.firstName() );
        formatStr( ownerInfo, licenseOwner.lastName() );
        formatStr( ownerInfo, licenseOwner.email() );
        formatStr( ownerInfo, licenseOwner.company() );
        if( !ownerInfo.empty() )
            std::cout << "Customer information (licensed to): " << ownerInfo << std::endl;
    }

    auto licenseUser = license->licenseUser();
    if( licenseUser )
    {
        std::string userInfo;
        formatStr( userInfo, licenseUser->firstName() );
        formatStr( userInfo, licenseUser->lastName() );
        formatStr( userInfo, licenseUser->email() );
        if( !userInfo.empty() )
            std::cout << "License user information: " << userInfo << std::endl;
    }

    if( !license->key().empty() )
        std::cout << "Key = " << license->key() << std::endl;

    if( !license->user().empty() )
        std::cout << "User = " << license->user() << std::endl;
    
    std::cout << "Type = " << license->type().toFormattedString() << std::endl;
    std::cout << "Status = " << license->status() << std::endl;
    std::cout << "IsActive = " << license->isActive() << std::endl;
    std::cout << "IsEnabled = " << license->isEnabled() << std::endl;
    std::cout << "IsTrial = " << license->isTrial() << std::endl;
    std::cout << "IsFloating = " << license->isFloating() << std::endl;
    std::cout << "Trial period for current license = " << license->trialPeriod() << std::endl;
    if( license->isFloating() )
    {
        std::cout << "Current floating slots count = " << license->floatingInUseCount() << std::endl;
        std::cout << "Overall floating slots count = " << license->maxFloatingUsers() << std::endl;
        auto endDate = TmToStr( license->floatingEndDateTime() );
        if( license->isBorrowed() )
            std::cout << "The license is borrowed until: " << endDate << std::endl;
        else
        {
            std::cout << "Registration of this floating license expires at: " << endDate << std::endl;
            if( license->maxBorrowTime() > 0 )
                std::cout << "License can be borrowed for " << license->maxBorrowTime()
                          << " hours max" << std::endl;
            else
                std::cout << "License borrowing is not allowed" << std::endl;
        }
    }
    std::cout << "IsOfflineActivated = " << license->isOfflineActivated() << std::endl;
    std::cout << "Times activated = " << license->timesActivated() << std::endl;
    std::cout << "Max activations = " << license->maxActivations() << std::endl;
    std::cout << "Transfer count = " << license->transferCount() << std::endl;

    if( license->isDeviceTransferAllowed() )
    {
        if( license->isDeviceTransferLimited() )
            std::cout << "Device transfer limit = " << license->transferLimit() << std::endl;
        else
            std::cout << "This license has unlimited device transfers" << std::endl;
    }
    else
        std::cout << "Device transfer is not allowed" << std::endl;

    if( !license->startDate().empty() )
        std::cout << "Start date = " << license->startDate() << std::endl;
    std::cout << "Validity Period = " << TmToStr( license->validityPeriod() ) << std::endl;
    std::cout << "Validity Period UTC = " << TmToStr( license->validityPeriodUtc() ) << std::endl;
    std::cout << "Days remaining till license expires = " << license->daysRemaining() << std::endl;
    if( license->type() == LicenseTypeSubscription )
        std::cout << "Subscription grace period = " << license->subscriptionGracePeriod() << std::endl;
    std::cout << "Maintenance period = " << TmToStr( license->maintenancePeriod() ) << std::endl;
    std::cout << "Maintenance period UTC = " << TmToStr( license->maintenancePeriodUtc() ) <<std::endl;
    std::cout << "Maintenance days remaining = " << license->maintenanceDaysRemaining() << std::endl;
    std::cout << "Last online check date = " << TmToStr( license->lastCheckDate() ) << std::endl;
    std::cout << "Days passed since last online check = " << license->daysPassedSinceLastCheck() << std::endl;
    std::cout << "Metadata = " << license->metadata() << std::endl;

    auto productFeatures = license->features();
    if( !productFeatures.empty() )
    {
        std::cout << "Product features available for this license:" << std::endl;
        for( auto feature : productFeatures )
            std::cout << feature.toString() << std::endl;
    }

    auto dataFields = license->customFields();
    if( !dataFields.empty() )
    {
        std::cout << "Custom data fields available for this license:" << std::endl;
        for( const auto& field : dataFields )
            std::cout << "Data field - Name: " << field.fieldName() << ", Value: " << field.fieldValue() << std::endl;
    }

    const auto& userData = license->userData();
    if( !userData.empty() )
    {
        std::cout << "User data for this license: " << std::endl;
        for( const auto& field : userData )
            std::cout << "Data field - Name: " << field.fieldName() << ", Value: " << field.fieldValue() << std::endl;
    }

    if( license->type() == LicenseTypeConsumption )
    {
        std::cout << "Total consumptions = " << license->totalConsumption() << std::endl;
        if( license->isUnlimitedConsumptionAllowed() )
            std::cout << "Max consumptions = " << "Unlimited" << std::endl;
        else
            std::cout << "Max consumptions = " << license->maxConsumption() << std::endl;
        std::cout << "Is overages allowed = " << license->isOveragesAllowed() << std::endl;
        if( license->isOveragesAllowed() )
            std::cout << "Max overages = " << license->maxOverages() << std::endl;
    }
}
