/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2016 BMW Car IT GmbH
 * %%
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * #L%
 */

#include "LocalDomainAccessController.h"
#include "LocalDomainAccessStore.h"
#include "joynr/infrastructure/GlobalDomainAccessControllerProxy.h"

#include "joynr/infrastructure/DacTypes/DomainRoleEntry.h"
#include "joynr/infrastructure/GlobalDomainAccessControllerDomainRoleEntryChangedBroadcastFilterParameters.h"
#include "joynr/infrastructure/GlobalDomainAccessControllerMasterAccessControlEntryChangedBroadcastFilterParameters.h"
#include "joynr/infrastructure/GlobalDomainAccessControllerOwnerAccessControlEntryChangedBroadcastFilterParameters.h"
#include "joynr/infrastructure/GlobalDomainAccessControllerMediatorAccessControlEntryChangedBroadcastFilterParameters.h"
#include "joynr/OnChangeSubscriptionQos.h"
#include "joynr/TypeUtil.h"

#include <cassert>
#include <atomic>
#include <tuple>

namespace joynr
{

using namespace infrastructure;
using namespace infrastructure::DacTypes;

INIT_LOGGER(LocalDomainAccessController);

std::chrono::milliseconds LocalDomainAccessController::broadcastMinInterval =
        std::chrono::seconds(1);
// 10 years
std::chrono::milliseconds LocalDomainAccessController::broadcastSubscriptionValidity =
        std::chrono::hours(24) * 365 * 10;
std::chrono::milliseconds LocalDomainAccessController::broadcastPublicationTtl =
        std::chrono::seconds(5);

//--- Declarations of nested classes -------------------------------------------

class LocalDomainAccessController::Initialiser
{
public:
    Initialiser(LocalDomainAccessController& parent,
                const std::string& domain,
                const std::string& interfaceName);

    // Called to indicate that some data has been initialised
    void update();

    // Called to abort the initialisation
    void abort();

private:
    std::atomic<std::uint8_t> counter;
    std::atomic<bool> aborted;
    LocalDomainAccessController& parent;
    std::string domain;
    std::string interfaceName;
};

class LocalDomainAccessController::DomainRoleEntryChangedBroadcastListener
        : public ISubscriptionListener<infrastructure::DacTypes::ChangeType::Enum,
                                       infrastructure::DacTypes::DomainRoleEntry>
{
public:
    explicit DomainRoleEntryChangedBroadcastListener(LocalDomainAccessController& parent);
    void onReceive(const infrastructure::DacTypes::ChangeType::Enum& changeType,
                   const infrastructure::DacTypes::DomainRoleEntry& changedDre) override;
    void onError(const exceptions::JoynrRuntimeException& error) override;

private:
    LocalDomainAccessController& parent;
};

class LocalDomainAccessController::MasterAccessControlEntryChangedBroadcastListener
        : public ISubscriptionListener<infrastructure::DacTypes::ChangeType::Enum,
                                       infrastructure::DacTypes::MasterAccessControlEntry>
{
public:
    explicit MasterAccessControlEntryChangedBroadcastListener(LocalDomainAccessController& parent);
    void onReceive(
            const infrastructure::DacTypes::ChangeType::Enum& changeType,
            const infrastructure::DacTypes::MasterAccessControlEntry& changedMasterAce) override;
    void onError(const exceptions::JoynrRuntimeException& error) override;

private:
    LocalDomainAccessController& parent;
};

class LocalDomainAccessController::MediatorAccessControlEntryChangedBroadcastListener
        : public ISubscriptionListener<infrastructure::DacTypes::ChangeType::Enum,
                                       infrastructure::DacTypes::MasterAccessControlEntry>
{
public:
    explicit MediatorAccessControlEntryChangedBroadcastListener(
            LocalDomainAccessController& parent);
    void onReceive(
            const infrastructure::DacTypes::ChangeType::Enum& changeType,
            const infrastructure::DacTypes::MasterAccessControlEntry& changedMediatorAce) override;
    void onError(const exceptions::JoynrRuntimeException& error) override;

private:
    LocalDomainAccessController& parent;
};

class LocalDomainAccessController::OwnerAccessControlEntryChangedBroadcastListener
        : public ISubscriptionListener<infrastructure::DacTypes::ChangeType::Enum,
                                       infrastructure::DacTypes::OwnerAccessControlEntry>
{
public:
    explicit OwnerAccessControlEntryChangedBroadcastListener(LocalDomainAccessController& parent);
    void onReceive(
            const infrastructure::DacTypes::ChangeType::Enum& changeType,
            const infrastructure::DacTypes::OwnerAccessControlEntry& changedOwnerAce) override;
    void onError(const exceptions::JoynrRuntimeException& error) override;

private:
    LocalDomainAccessController& parent;
};

//--- LocalDomainAccessController ----------------------------------------------

LocalDomainAccessController::LocalDomainAccessController(
        LocalDomainAccessStore* localDomainAccessStore)
        : accessControlAlgorithm(),
          dreSubscriptions(),
          aceSubscriptions(),
          globalDomainAccessControllerProxy(),
          localDomainAccessStore(localDomainAccessStore),
          consumerPermissionRequests(),
          initStateMutex(),
          domainRoleEntryChangedBroadcastListener(
                  new DomainRoleEntryChangedBroadcastListener(*this)),
          masterAccessControlEntryChangedBroadcastListener(
                  new MasterAccessControlEntryChangedBroadcastListener(*this)),
          mediatorAccessControlEntryChangedBroadcastListener(
                  new MediatorAccessControlEntryChangedBroadcastListener(*this)),
          ownerAccessControlEntryChangedBroadcastListener(
                  new OwnerAccessControlEntryChangedBroadcastListener(*this))
{
}

LocalDomainAccessController::~LocalDomainAccessController()
{
    delete localDomainAccessStore;
}

void LocalDomainAccessController::init(
        std::shared_ptr<GlobalDomainAccessControllerProxy> globalDomainAccessControllerProxy)
{
    this->globalDomainAccessControllerProxy = globalDomainAccessControllerProxy;
}

bool LocalDomainAccessController::hasRole(const std::string& userId,
                                          const std::string& domain,
                                          Role::Enum role)
{
    JOYNR_LOG_DEBUG(logger, "execute: entering hasRole");

    // See if the user has the given role
    bool hasRole = false;
    Optional<DomainRoleEntry> dre = localDomainAccessStore->getDomainRole(userId, role);
    if (dre) {
        std::vector<std::string> domains = dre.getValue().getDomains();
        if (vectorContains(domains, domain)) {
            hasRole = true;
        }
    }

    // Subscribe changes in the users roles
    if (!dreSubscriptions.count(userId)) {
        dreSubscriptions.insert(std::make_pair(userId, subscribeForDreChange(userId)));
    }

    return hasRole;
}

void LocalDomainAccessController::getConsumerPermission(
        const std::string& userId,
        const std::string& domain,
        const std::string& interfaceName,
        TrustLevel::Enum trustLevel,
        std::shared_ptr<IGetConsumerPermissionCallback> callback)
{
    JOYNR_LOG_DEBUG(logger, "Entering getConsumerPermission with unknown operation");

    // Is the ACL for this domain/interface available?
    std::string compoundKey = createCompoundKey(domain, interfaceName);
    bool needsInit = false;
    {
        std::lock_guard<std::mutex> lock(initStateMutex);
        if (!aceSubscriptions.count(compoundKey)) {
            // Queue the request
            ConsumerPermissionRequest request = {
                    userId, domain, interfaceName, trustLevel, callback};

            if (queueConsumerRequest(compoundKey, request)) {
                // There are requests already queued - do nothing
                return;
            } else {
                // There are no requests already queued - further
                // initialisation is needed
                needsInit = true;
            }
        }
    }

    // Do further initialisation outside of the mutex to prevent deadlocks
    if (needsInit) {
        // Get the data for this domain interface and do not wait for it
        initialiseLocalDomainAccessStore(userId, domain, interfaceName);
        return;
    }

    // If this point is reached the data for the ACL check is available
    std::vector<MasterAccessControlEntry> masterAces =
            localDomainAccessStore->getMasterAccessControlEntries(userId, domain, interfaceName);
    std::vector<MasterAccessControlEntry> mediatorAces =
            localDomainAccessStore->getMediatorAccessControlEntries(userId, domain, interfaceName);
    std::vector<OwnerAccessControlEntry> ownerAces =
            localDomainAccessStore->getOwnerAccessControlEntries(userId, domain, interfaceName);

    // The operations of the ACEs should only contain wildcards, if not
    // getConsumerPermission should be called with an operation
    if (!(onlyWildcardOperations(masterAces) && onlyWildcardOperations(mediatorAces) &&
          onlyWildcardOperations(ownerAces))) {
        callback->operationNeeded();
    } else {
        // The operations are all wildcards
        Permission::Enum permission = getConsumerPermission(
                userId, domain, interfaceName, LocalDomainAccessStore::WILDCARD, trustLevel);
        callback->consumerPermission(permission);
    }
}

Permission::Enum LocalDomainAccessController::getConsumerPermission(
        const std::string& userId,
        const std::string& domain,
        const std::string& interfaceName,
        const std::string& operation,
        TrustLevel::Enum trustLevel)
{
    JOYNR_LOG_DEBUG(logger, "Entering getConsumerPermission with known operation");

    Optional<MasterAccessControlEntry> masterAceOptional =
            localDomainAccessStore->getMasterAccessControlEntry(
                    userId, domain, interfaceName, operation);
    Optional<MasterAccessControlEntry> mediatorAceOptional =
            localDomainAccessStore->getMediatorAccessControlEntry(
                    userId, domain, interfaceName, operation);
    Optional<OwnerAccessControlEntry> ownerAceOptional =
            localDomainAccessStore->getOwnerAccessControlEntry(
                    userId, domain, interfaceName, operation);

    return accessControlAlgorithm.getConsumerPermission(
            masterAceOptional, mediatorAceOptional, ownerAceOptional, trustLevel);
}

std::vector<MasterAccessControlEntry> LocalDomainAccessController::getMasterAccessControlEntries(
        const std::string& uid)
{
    std::vector<MasterAccessControlEntry> resultMasterAces;
    globalDomainAccessControllerProxy->getMasterAccessControlEntries(resultMasterAces, uid);

    return resultMasterAces;
}

std::vector<MasterAccessControlEntry> LocalDomainAccessController::
        getEditableMasterAccessControlEntries(const std::string& uid)
{
    std::vector<MasterAccessControlEntry> resultMasterAces;
    globalDomainAccessControllerProxy->getEditableMasterAccessControlEntries(resultMasterAces, uid);

    return resultMasterAces;
}

bool LocalDomainAccessController::updateMasterAccessControlEntry(
        const MasterAccessControlEntry& updatedMasterAce)
{
    bool success;
    globalDomainAccessControllerProxy->updateMasterAccessControlEntry(success, updatedMasterAce);

    return success;
}

bool LocalDomainAccessController::removeMasterAccessControlEntry(const std::string& uid,
                                                                 const std::string& domain,
                                                                 const std::string& interfaceName,
                                                                 const std::string& operation)
{
    bool success;
    globalDomainAccessControllerProxy->removeMasterAccessControlEntry(
            success, uid, domain, interfaceName, operation);

    return success;
}

std::vector<MasterAccessControlEntry> LocalDomainAccessController::getMediatorAccessControlEntries(
        const std::string& uid)
{
    std::vector<MasterAccessControlEntry> resultMediatorAces;
    globalDomainAccessControllerProxy->getMediatorAccessControlEntries(resultMediatorAces, uid);

    return resultMediatorAces;
}

std::vector<MasterAccessControlEntry> LocalDomainAccessController::
        getEditableMediatorAccessControlEntries(const std::string& uid)
{
    std::vector<MasterAccessControlEntry> resultMediatorAces;
    globalDomainAccessControllerProxy->getEditableMediatorAccessControlEntries(
            resultMediatorAces, uid);

    return resultMediatorAces;
}

bool LocalDomainAccessController::updateMediatorAccessControlEntry(
        const MasterAccessControlEntry& updatedMediatorAce)
{
    bool success;
    globalDomainAccessControllerProxy->updateMediatorAccessControlEntry(
            success, updatedMediatorAce);

    return success;
}

bool LocalDomainAccessController::removeMediatorAccessControlEntry(const std::string& uid,
                                                                   const std::string& domain,
                                                                   const std::string& interfaceName,
                                                                   const std::string& operation)
{
    bool success;
    globalDomainAccessControllerProxy->removeMediatorAccessControlEntry(
            success, uid, domain, interfaceName, operation);

    return success;
}

std::vector<OwnerAccessControlEntry> LocalDomainAccessController::getOwnerAccessControlEntries(
        const std::string& uid)
{
    std::vector<OwnerAccessControlEntry> resultOwnerAces;
    globalDomainAccessControllerProxy->getOwnerAccessControlEntries(resultOwnerAces, uid);

    return resultOwnerAces;
}

std::vector<OwnerAccessControlEntry> LocalDomainAccessController::
        getEditableOwnerAccessControlEntries(const std::string& uid)
{
    std::vector<OwnerAccessControlEntry> resultOwnerAces;
    globalDomainAccessControllerProxy->getEditableOwnerAccessControlEntries(resultOwnerAces, uid);

    return resultOwnerAces;
}

bool LocalDomainAccessController::updateOwnerAccessControlEntry(
        const OwnerAccessControlEntry& updatedOwnerAce)
{
    bool success;
    globalDomainAccessControllerProxy->updateOwnerAccessControlEntry(success, updatedOwnerAce);

    return success;
}

bool LocalDomainAccessController::removeOwnerAccessControlEntry(const std::string& uid,
                                                                const std::string& domain,
                                                                const std::string& interfaceName,
                                                                const std::string& operation)
{
    bool success;
    globalDomainAccessControllerProxy->removeOwnerAccessControlEntry(
            success, uid, domain, interfaceName, operation);

    return success;
}

Permission::Enum LocalDomainAccessController::getProviderPermission(
        const std::string& uid,
        const std::string& domain,
        const std::string& interfaceName,
        TrustLevel::Enum trustLevel)
{
    assert(false && "Not implemented yet");
    std::ignore = uid;
    std::ignore = domain;
    std::ignore = interfaceName;

    return accessControlAlgorithm.getProviderPermission(
            Optional<MasterAccessControlEntry>::createNull(),
            Optional<MasterAccessControlEntry>::createNull(),
            Optional<OwnerAccessControlEntry>::createNull(),
            trustLevel);
}

std::vector<MasterRegistrationControlEntry> LocalDomainAccessController::
        getMasterRegistrationControlEntries(const std::string& uid)
{
    assert(false && "Not implemented yet");
    std::ignore = uid;

    return std::vector<MasterRegistrationControlEntry>();
}

//---- Unused methods copied from Java implementation --------------------------

std::vector<MasterRegistrationControlEntry> LocalDomainAccessController::
        getEditableMasterRegistrationControlEntries(const std::string& uid)
{
    assert(false && "Not implemented yet");
    std::ignore = uid;

    return std::vector<MasterRegistrationControlEntry>();
}

bool LocalDomainAccessController::updateMasterRegistrationControlEntry(
        const MasterRegistrationControlEntry& updatedMasterRce)
{
    assert(false && "Not implemented yet");
    std::ignore = updatedMasterRce;

    return false;
}

bool LocalDomainAccessController::removeMasterRegistrationControlEntry(
        const std::string& uid,
        const std::string& domain,
        const std::string& interfaceName)
{
    assert(false && "Not implemented yet");
    std::ignore = uid;
    std::ignore = domain;
    std::ignore = interfaceName;

    return false;
}

std::vector<MasterRegistrationControlEntry> LocalDomainAccessController::
        getMediatorRegistrationControlEntries(const std::string& uid)
{
    assert(false && "Not implemented yet");
    std::ignore = uid;

    return std::vector<MasterRegistrationControlEntry>();
}

std::vector<MasterRegistrationControlEntry> LocalDomainAccessController::
        getEditableMediatorRegistrationControlEntries(const std::string& uid)
{
    assert(false && "Not implemented yet");
    std::ignore = uid;

    return std::vector<MasterRegistrationControlEntry>();
}

bool LocalDomainAccessController::updateMediatorRegistrationControlEntry(
        const MasterRegistrationControlEntry& updatedMediatorRce)
{
    assert(false && "Not implemented yet");
    std::ignore = updatedMediatorRce;

    return false;
}

bool LocalDomainAccessController::removeMediatorRegistrationControlEntry(
        const std::string& uid,
        const std::string& domain,
        const std::string& interfaceName)
{
    assert(false && "Not implemented yet");
    std::ignore = uid;
    std::ignore = domain;
    std::ignore = interfaceName;

    return false;
}

std::vector<OwnerRegistrationControlEntry> LocalDomainAccessController::
        getOwnerRegistrationControlEntries(const std::string& uid)
{
    assert(false && "Not implemented yet");
    std::ignore = uid;

    return std::vector<OwnerRegistrationControlEntry>();
}

std::vector<OwnerRegistrationControlEntry> LocalDomainAccessController::
        getEditableOwnerRegistrationControlEntries(const std::string& uid)
{
    assert(false && "Not implemented yet");
    std::ignore = uid;

    return std::vector<OwnerRegistrationControlEntry>();
}

bool LocalDomainAccessController::updateOwnerRegistrationControlEntry(
        const OwnerRegistrationControlEntry& updatedOwnerRce)
{
    assert(false && "Not implemented yet");
    std::ignore = updatedOwnerRce;

    return false;
}

bool LocalDomainAccessController::removeOwnerRegistrationControlEntry(
        const std::string& uid,
        const std::string& domain,
        const std::string& interfaceName)
{
    assert(false && "Not implemented yet");
    std::ignore = uid;
    std::ignore = domain;
    std::ignore = interfaceName;

    return false;
}

void LocalDomainAccessController::unregisterProvider(const std::string& domain,
                                                     const std::string& interfaceName)
{
    // Get the subscription ids
    std::string compoundKey = createCompoundKey(domain, interfaceName);
    AceSubscription subscriptionIds;
    {
        std::lock_guard<std::mutex> lock(initStateMutex);
        if (!aceSubscriptions.count(compoundKey)) {
            return;
        }
        subscriptionIds = aceSubscriptions[compoundKey];
    }

    JOYNR_LOG_DEBUG(logger,
                    "Unsubscribing from ACL broadcasts for domain {}, interface {}",
                    domain,
                    interfaceName);

    // Unsubscribe from ACE change subscriptions
    globalDomainAccessControllerProxy->unsubscribeFromMasterAccessControlEntryChangedBroadcast(
            subscriptionIds.masterAceSubscriptionId);
    globalDomainAccessControllerProxy->unsubscribeFromMediatorAccessControlEntryChangedBroadcast(
            subscriptionIds.mediatorAceSubscriptionId);
    globalDomainAccessControllerProxy->unsubscribeFromOwnerAccessControlEntryChangedBroadcast(
            subscriptionIds.ownerAceSubscriptionId);
}

// Initialise the data for the given data/interface. This function is non-blocking
void LocalDomainAccessController::initialiseLocalDomainAccessStore(const std::string& userId,
                                                                   const std::string& domain,
                                                                   const std::string& interfaceName)
{
    // Create an object to keep track of the initialisation
    std::shared_ptr<Initialiser> initialiser(new Initialiser(*this, domain, interfaceName));

    // Initialise domain roles from global data
    // TODO: confirm that this is needed
    std::function<void(const std::vector<DomainRoleEntry>& domainRoleEntries)> domainRoleOnSuccess =
            [this, initialiser](const std::vector<DomainRoleEntry>& domainRoleEntries) {
        // Add the results
        for (const DomainRoleEntry& dre : domainRoleEntries) {
            localDomainAccessStore->updateDomainRole(dre);
        }
        initialiser->update();
    };

    std::function<void(const exceptions::JoynrException&)> domainRoleOnError =
            [this, initialiser](const exceptions::JoynrException& error) {
        JOYNR_LOG_ERROR(logger,
                        "Aborting ACL initialisation due to communication error:\n{}",
                        error.getMessage());

        // Abort the initialisation
        initialiser->abort();
    };

    globalDomainAccessControllerProxy->getDomainRolesAsync(
            userId, domainRoleOnSuccess, domainRoleOnError);

    std::function<void(const std::vector<MasterAccessControlEntry>& masterAces)>
            masterAceOnSuccess =
                    [this, initialiser](const std::vector<MasterAccessControlEntry>& masterAces) {
        // Add the results
        for (const MasterAccessControlEntry& masterAce : masterAces) {
            localDomainAccessStore->updateMasterAccessControlEntry(masterAce);
        }
        initialiser->update();
    };

    std::function<void(const exceptions::JoynrException& error)> masterAceOnError =
            [this, initialiser](const exceptions::JoynrException& error) {
        JOYNR_LOG_ERROR(logger,
                        "Aborting ACL initialisation due to communication error:\n{}",
                        error.getMessage());

        // Abort the initialisation
        initialiser->abort();
    };

    globalDomainAccessControllerProxy->getMasterAccessControlEntriesAsync(
            domain, interfaceName, masterAceOnSuccess, masterAceOnError);

    // Initialise mediator access control entries from global data
    std::function<void(const std::vector<MasterAccessControlEntry>& mediatorAces)>
            mediatorAceOnSuccess =
                    [this, initialiser](const std::vector<MasterAccessControlEntry>& mediatorAces) {
        // Add the results
        for (const MasterAccessControlEntry& mediatorAce : mediatorAces) {
            localDomainAccessStore->updateMediatorAccessControlEntry(mediatorAce);
        }
        initialiser->update();
    };

    std::function<void(const exceptions::JoynrException& error)> mediatorAceOnError =
            [this, initialiser](const exceptions::JoynrException& error) {
        JOYNR_LOG_ERROR(logger,
                        "Aborting ACL initialisation due to communication error:\n{}",
                        error.getMessage());

        // Abort the initialisation
        initialiser->abort();
    };

    globalDomainAccessControllerProxy->getMediatorAccessControlEntriesAsync(
            domain, interfaceName, mediatorAceOnSuccess, mediatorAceOnError);

    // Initialise owner access control entries from global data
    std::function<void(const std::vector<OwnerAccessControlEntry>& ownerAces)> ownerAceOnSuccess =
            [this, initialiser](const std::vector<OwnerAccessControlEntry>& ownerAces) {
        // Add the results
        for (const OwnerAccessControlEntry& ownerAce : ownerAces) {
            localDomainAccessStore->updateOwnerAccessControlEntry(ownerAce);
        }
        initialiser->update();
    };

    std::function<void(const exceptions::JoynrException& error)> ownerAceOnError =
            [this, initialiser](const exceptions::JoynrException& error) {
        JOYNR_LOG_ERROR(logger,
                        "Aborting ACL initialisation due to communication error:\n{}",
                        error.getMessage());

        // Abort the initialisation
        initialiser->abort();
    };

    globalDomainAccessControllerProxy->getOwnerAccessControlEntriesAsync(
            domain, interfaceName, ownerAceOnSuccess, ownerAceOnError);
}

// Called when the data for the given domain/interface has been obtained from the GDAC
void LocalDomainAccessController::initialised(const std::string& domain,
                                              const std::string& interfaceName)
{
    std::string compoundKey = createCompoundKey(domain, interfaceName);
    std::vector<ConsumerPermissionRequest> requests;

    {
        std::lock_guard<std::mutex> lock(initStateMutex);

        // Subscribe to ACL broadcasts about this domain/interface
        aceSubscriptions.insert(
                std::make_pair(compoundKey, subscribeForAceChange(domain, interfaceName)));

        // Remove requests for processing
        auto it = consumerPermissionRequests.find(compoundKey);
        requests = it->second;
        consumerPermissionRequests.erase(it);
    }

    // Handle any queued requests for this domain/interface
    processConsumerRequests(requests);
}

void LocalDomainAccessController::abortInitialisation(const std::string& domain,
                                                      const std::string& interfaceName)
{
    JOYNR_LOG_INFO(logger,
                   "Removing outstanding ACL requests for domain {}, interface {}",
                   domain,
                   interfaceName);

    std::string compoundKey = createCompoundKey(domain, interfaceName);
    std::vector<ConsumerPermissionRequest> requests;

    {
        std::lock_guard<std::mutex> lock(initStateMutex);

        // Remove requests that cannot be processed
        auto it = consumerPermissionRequests.find(compoundKey);
        requests = it->second;
        consumerPermissionRequests.erase(it);
    }

    // Mark all the requests as failed - we have no information from the Global
    // Domain Access Controller
    for (const ConsumerPermissionRequest& request : requests) {
        request.callbacks->consumerPermission(Permission::NO);
    }
}

// Returns true if other requests have already been queued
bool LocalDomainAccessController::queueConsumerRequest(const std::string& key,
                                                       const ConsumerPermissionRequest& request)
{
    // This function assumes that the initStateMutex has already been obtained

    if (consumerPermissionRequests.count(key)) {
        consumerPermissionRequests[key].push_back(request);
        return true;
    } else {
        std::vector<ConsumerPermissionRequest> requestList;
        requestList.push_back(request);
        consumerPermissionRequests.insert(std::make_pair(key, requestList));
        return false;
    }
}

void LocalDomainAccessController::processConsumerRequests(
        const std::vector<ConsumerPermissionRequest>& requests)
{
    for (const ConsumerPermissionRequest& request : requests) {
        getConsumerPermission(request.userId,
                              request.domain,
                              request.interfaceName,
                              request.trustLevel,
                              request.callbacks);
    }
}

std::string LocalDomainAccessController::subscribeForDreChange(const std::string& userId)
{
    OnChangeSubscriptionQos broadcastSubscriptionQos;
    broadcastSubscriptionQos.setMinInterval(broadcastMinInterval.count());
    broadcastSubscriptionQos.setValidity(broadcastSubscriptionValidity.count());
    broadcastSubscriptionQos.setPublicationTtl(broadcastPublicationTtl.count());
    GlobalDomainAccessControllerDomainRoleEntryChangedBroadcastFilterParameters
            domainRoleFilterParameters;
    domainRoleFilterParameters.setUserIdOfInterest(userId);

    std::string subscriptionId =
            globalDomainAccessControllerProxy->subscribeToDomainRoleEntryChangedBroadcast(
                    domainRoleFilterParameters,
                    std::static_pointer_cast<
                            ISubscriptionListener<ChangeType::Enum, DomainRoleEntry>>(
                            domainRoleEntryChangedBroadcastListener),
                    broadcastSubscriptionQos);

    return subscriptionId;
}

LocalDomainAccessController::AceSubscription LocalDomainAccessController::subscribeForAceChange(
        const std::string& domain,
        const std::string& interfaceName)
{
    OnChangeSubscriptionQos broadcastSubscriptionQos;

    broadcastSubscriptionQos.setMinInterval(broadcastMinInterval.count());
    broadcastSubscriptionQos.setValidity(broadcastSubscriptionValidity.count());
    broadcastSubscriptionQos.setPublicationTtl(broadcastPublicationTtl.count());

    AceSubscription subscriptionIds;

    GlobalDomainAccessControllerMasterAccessControlEntryChangedBroadcastFilterParameters
            masterAcefilterParameters;
    masterAcefilterParameters.setDomainOfInterest(domain);
    masterAcefilterParameters.setInterfaceOfInterest(interfaceName);
    subscriptionIds.masterAceSubscriptionId =
            globalDomainAccessControllerProxy->subscribeToMasterAccessControlEntryChangedBroadcast(
                    masterAcefilterParameters,
                    std::static_pointer_cast<
                            ISubscriptionListener<ChangeType::Enum, MasterAccessControlEntry>>(
                            masterAccessControlEntryChangedBroadcastListener),
                    broadcastSubscriptionQos);

    GlobalDomainAccessControllerMediatorAccessControlEntryChangedBroadcastFilterParameters
            mediatorAceFilterParameters;
    mediatorAceFilterParameters.setDomainOfInterest(domain);
    mediatorAceFilterParameters.setInterfaceOfInterest(interfaceName);
    subscriptionIds.mediatorAceSubscriptionId =
            globalDomainAccessControllerProxy
                    ->subscribeToMediatorAccessControlEntryChangedBroadcast(
                            mediatorAceFilterParameters,
                            std::static_pointer_cast<
                                    ISubscriptionListener<ChangeType::Enum,
                                                          MasterAccessControlEntry>>(
                                    mediatorAccessControlEntryChangedBroadcastListener),
                            broadcastSubscriptionQos);

    GlobalDomainAccessControllerOwnerAccessControlEntryChangedBroadcastFilterParameters
            ownerAceFilterParameters;
    ownerAceFilterParameters.setDomainOfInterest(domain);
    ownerAceFilterParameters.setInterfaceOfInterest(interfaceName);
    subscriptionIds.ownerAceSubscriptionId =
            globalDomainAccessControllerProxy->subscribeToOwnerAccessControlEntryChangedBroadcast(
                    ownerAceFilterParameters,
                    std::static_pointer_cast<
                            ISubscriptionListener<ChangeType::Enum, OwnerAccessControlEntry>>(
                            ownerAccessControlEntryChangedBroadcastListener),
                    broadcastSubscriptionQos);

    return subscriptionIds;
}

std::string LocalDomainAccessController::createCompoundKey(const std::string& domain,
                                                           const std::string& interfaceName)
{
    std::string subscriptionMapKey(domain);
    subscriptionMapKey.push_back('\x1e'); // ascii record separator
    subscriptionMapKey.insert(0, interfaceName);

    return subscriptionMapKey;
}

//--- Implementation of Initialiser --------------------------------------------

LocalDomainAccessController::Initialiser::Initialiser(LocalDomainAccessController& parent,
                                                      const std::string& domain,
                                                      const std::string& interfaceName)
        : counter(4), aborted(false), parent(parent), domain(domain), interfaceName(interfaceName)
// counter == 4, because there are 4 init operations (DRT, MasterACE, MediatorACE, OwnerACE)
{
}

void LocalDomainAccessController::Initialiser::update()
{
    std::uint8_t prevValue = counter--;
    if (prevValue == 1) {
        // Initialisation has finished
        if (aborted) {
            parent.abortInitialisation(domain, interfaceName);
        } else {
            parent.initialised(domain, interfaceName);
        }
    }
}

void LocalDomainAccessController::Initialiser::abort()
{
    aborted = true;
    update();
}

//--- Implementation of DomainRoleEntryChangedBroadcastListener ----------------

LocalDomainAccessController::DomainRoleEntryChangedBroadcastListener::
        DomainRoleEntryChangedBroadcastListener(LocalDomainAccessController& parent)
        : parent(parent)
{
}

void LocalDomainAccessController::DomainRoleEntryChangedBroadcastListener::onReceive(
        const ChangeType::Enum& changeType,
        const DomainRoleEntry& changedDre)
{
    if (changeType != ChangeType::REMOVE) {
        parent.localDomainAccessStore->updateDomainRole(changedDre);
    } else {
        parent.localDomainAccessStore->removeDomainRole(changedDre.getUid(), changedDre.getRole());
    }
    JOYNR_LOG_DEBUG(parent.logger, "Changed DRE: {}", changedDre.toString());
}

void LocalDomainAccessController::DomainRoleEntryChangedBroadcastListener::onError(
        const exceptions::JoynrRuntimeException& error)
{
    std::ignore = error;
    JOYNR_LOG_ERROR(parent.logger, "Change of DRE failed!");
}

LocalDomainAccessController::MasterAccessControlEntryChangedBroadcastListener::
        MasterAccessControlEntryChangedBroadcastListener(LocalDomainAccessController& parent)
        : parent(parent)
{
}

void LocalDomainAccessController::MasterAccessControlEntryChangedBroadcastListener::onReceive(
        const ChangeType::Enum& changeType,
        const MasterAccessControlEntry& changedMasterAce)
{
    if (changeType != ChangeType::REMOVE) {
        parent.localDomainAccessStore->updateMasterAccessControlEntry(changedMasterAce);
        JOYNR_LOG_DEBUG(parent.logger, "Changed MasterAce: {}", changedMasterAce.toString());
    } else {
        parent.localDomainAccessStore->removeMasterAccessControlEntry(
                changedMasterAce.getUid(),
                changedMasterAce.getDomain(),
                changedMasterAce.getInterfaceName(),
                changedMasterAce.getOperation());
        JOYNR_LOG_DEBUG(parent.logger, "Removed MasterAce: {}", changedMasterAce.toString());
    }
}

void LocalDomainAccessController::MasterAccessControlEntryChangedBroadcastListener::onError(
        const exceptions::JoynrRuntimeException& error)
{
    std::ignore = error;
    JOYNR_LOG_ERROR(parent.logger, "Change of MasterAce failed!");
}

LocalDomainAccessController::MediatorAccessControlEntryChangedBroadcastListener::
        MediatorAccessControlEntryChangedBroadcastListener(LocalDomainAccessController& parent)
        : parent(parent)
{
}

void LocalDomainAccessController::MediatorAccessControlEntryChangedBroadcastListener::onReceive(
        const ChangeType::Enum& changeType,
        const MasterAccessControlEntry& changedMediatorAce)
{
    if (changeType != ChangeType::REMOVE) {
        parent.localDomainAccessStore->updateMediatorAccessControlEntry(changedMediatorAce);
    } else {
        parent.localDomainAccessStore->removeMediatorAccessControlEntry(
                changedMediatorAce.getUid(),
                changedMediatorAce.getDomain(),
                changedMediatorAce.getInterfaceName(),
                changedMediatorAce.getOperation());
    }
    JOYNR_LOG_DEBUG(parent.logger, "Changed MediatorAce: {}", changedMediatorAce.toString());
}

void LocalDomainAccessController::MediatorAccessControlEntryChangedBroadcastListener::onError(
        const exceptions::JoynrRuntimeException& error)
{
    std::ignore = error;
    JOYNR_LOG_ERROR(parent.logger, "Change of MediatorAce failed!");
}

LocalDomainAccessController::OwnerAccessControlEntryChangedBroadcastListener::
        OwnerAccessControlEntryChangedBroadcastListener(LocalDomainAccessController& parent)
        : parent(parent)
{
}

void LocalDomainAccessController::OwnerAccessControlEntryChangedBroadcastListener::onReceive(
        const ChangeType::Enum& changeType,
        const OwnerAccessControlEntry& changedOwnerAce)
{
    if (changeType != ChangeType::REMOVE) {
        parent.localDomainAccessStore->updateOwnerAccessControlEntry(changedOwnerAce);
    } else {
        parent.localDomainAccessStore->removeOwnerAccessControlEntry(
                changedOwnerAce.getUid(),
                changedOwnerAce.getDomain(),
                changedOwnerAce.getInterfaceName(),
                changedOwnerAce.getOperation());
    }
    JOYNR_LOG_DEBUG(parent.logger, "Changed OwnerAce: {}", changedOwnerAce.toString());
}

void LocalDomainAccessController::OwnerAccessControlEntryChangedBroadcastListener::onError(
        const exceptions::JoynrRuntimeException& error)
{
    std::ignore = error;
    JOYNR_LOG_ERROR(parent.logger, "Change of OwnerAce failed!");
}

template <typename T>
bool LocalDomainAccessController::onlyWildcardOperations(const std::vector<T>& aceEntries)
{
    if (aceEntries.empty()) {
        return true;
    }

    if (aceEntries.size() > 1) {
        return false;
    }

    return aceEntries.begin()->getOperation() == LocalDomainAccessStore::WILDCARD;
}

} // namespace joynr
