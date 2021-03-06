/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2013 BMW Car IT GmbH
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
#ifndef MY_PROVIDER_H
#define MY_PROVIDER_H

#include "joynr/interlanguagetest/DefaultTestInterfaceProvider.h"
#if 0
#include "joynr/vehicle/QtTestInterfaceStation.h"
#include "joynr/vehicle/QtCountry.h"
#endif
//#include "joynr/joynrlogging.h"
#include "joynr/Logger.h"
#if 0
#include <QList>
#include <QMap>
#include <QMutex>
#endif

/**
  * A TestInterface Provider with a circular list of radio stations
  */
class IltProvider : public joynr::interlanguagetest::DefaultTestInterfaceProvider
{
public:
    IltProvider();
    ~IltProvider();

    /*
     * Implementation of Getters for Franca attributes
     */

    /*
     * Implementation of Franca methods
     */

    virtual void methodWithoutParameters(
            std::function<void()> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void methodWithoutInputParameter(
            std::function<void(const bool& booleanOut)> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void methodWithoutOutputParameter(
            const bool& booleanArg,
            std::function<void()> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void methodWithSinglePrimitiveParameters(
            const uint16_t& uInt16Arg,
            std::function<void(const std::string& stringOut)> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void methodWithMultiplePrimitiveParameters(
            const int32_t& int32Arg,
            const float& floatArg,
            const bool& booleanArg,
            std::function<void(const double& doubleOut, const std::string& stringOut)> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void methodWithSingleArrayParameters(
            const std::vector<double>& doubleArrayArg,
            std::function<void(const std::vector<std::string>& stringArrayOut)> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void methodWithMultipleArrayParameters(
            const std::vector<std::string>& stringArrayArg,
            const std::vector<int8_t>& int8ArrayArg,
            const std::vector<joynr::interlanguagetest::namedTypeCollection2::
                                      ExtendedInterfaceEnumerationInTypeCollection::Enum>&
                    enumArrayArg,
            const std::vector<
                    joynr::interlanguagetest::namedTypeCollection1::StructWithStringArray>&
                    structWithStringArrayArrayArg,
            std::function<void(
                    const std::vector<uint64_t>& uInt64ArrayOut,
                    const std::vector<
                            joynr::interlanguagetest::namedTypeCollection1::StructWithStringArray>&
                            structWithStringArrayArrayOut)> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void methodWithSingleEnumParameters(
            const joynr::interlanguagetest::namedTypeCollection2::
                    ExtendedEnumerationWithPartlyDefinedValues::Enum& enumerationArg,
            std::function<void(const joynr::interlanguagetest::namedTypeCollection2::
                                       ExtendedTypeCollectionEnumerationInTypeCollection::Enum&
                                               enumerationOut)> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void methodWithMultipleEnumParameters(
            const joynr::interlanguagetest::Enumeration::Enum& enumerationArg,
            const joynr::interlanguagetest::namedTypeCollection2::
                    ExtendedTypeCollectionEnumerationInTypeCollection::Enum& extendedEnumerationArg,
            std::function<void(const joynr::interlanguagetest::namedTypeCollection2::
                                       ExtendedEnumerationWithPartlyDefinedValues::Enum&
                                               extendedEnumerationOut,
                               const joynr::interlanguagetest::Enumeration::Enum& enumerationOut)>
                    onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void methodWithSingleStructParameters(
            const joynr::interlanguagetest::namedTypeCollection2::ExtendedBaseStruct&
                    extendedBaseStructArg,
            std::function<void(const joynr::interlanguagetest::namedTypeCollection2::
                                       ExtendedStructOfPrimitives& extendedStructOfPrimitivesOut)>
                    onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void methodWithMultipleStructParameters(
            const joynr::interlanguagetest::namedTypeCollection2::ExtendedStructOfPrimitives&
                    extendedStructOfPrimitivesArg,
            const joynr::interlanguagetest::namedTypeCollection2::BaseStruct& baseStructArg,
            std::function<void(
                    const joynr::interlanguagetest::namedTypeCollection2::BaseStructWithoutElements&
                            baseStructWithoutElementsOut,
                    const joynr::interlanguagetest::namedTypeCollection2::
                            ExtendedExtendedBaseStruct& extendedExtendedBaseStructOut)> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void overloadedMethod(
            std::function<void(const std::string& stringOut)> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void overloadedMethod(
            const bool& booleanArg,
            std::function<void(const std::string& stringOut)> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void overloadedMethod(
            const std::vector<joynr::interlanguagetest::namedTypeCollection2::
                                      ExtendedExtendedEnumeration::Enum>& enumArrayArg,
            const int64_t& int64Arg,
            const joynr::interlanguagetest::namedTypeCollection2::BaseStruct& baseStructArg,
            const bool& booleanArg,
            std::function<
                    void(const double& doubleOut,
                         const std::vector<std::string>& stringArrayOut,
                         const joynr::interlanguagetest::namedTypeCollection2::ExtendedBaseStruct&
                                 extendedBaseStructOut)> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void overloadedMethodWithSelector(
            std::function<void(const std::string& stringOut)> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void overloadedMethodWithSelector(
            const bool& booleanArg,
            std::function<void(const std::string& stringOut)> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void overloadedMethodWithSelector(
            const std::vector<joynr::interlanguagetest::namedTypeCollection2::
                                      ExtendedExtendedEnumeration::Enum>& enumArrayArg,
            const int64_t& int64Arg,
            const joynr::interlanguagetest::namedTypeCollection2::BaseStruct& baseStructArg,
            const bool& booleanArg,
            std::function<
                    void(const double& doubleOut,
                         const std::vector<std::string>& stringArrayOut,
                         const joynr::interlanguagetest::namedTypeCollection2::ExtendedBaseStruct&
                                 extendedBaseStructOut)> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void methodWithStringsAndSpecifiedStringOutLength(
            const std::string& stringArg,
            const int32_t& int32StringLengthArg,
            std::function<void(const std::string& stringOut)> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void methodWithoutErrorEnum(
            const std::string& wantedExceptionArg,
            std::function<void()> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException&)> onError);

    virtual void methodWithAnonymousErrorEnum(
            const std::string& wantedExceptionArg,
            std::function<void()> onSuccess,
            std::function<void(const joynr::interlanguagetest::TestInterface::
                                       MethodWithAnonymousErrorEnumErrorEnum::Enum& errorEnum)>
                    onError);

    virtual void methodWithExistingErrorEnum(
            const std::string& wantedExceptionArg,
            std::function<void()> onSuccess,
            std::function<void(const joynr::interlanguagetest::namedTypeCollection2::
                                       ExtendedErrorEnumTc::Enum& errorEnum)> onError);

    virtual void methodWithExtendedErrorEnum(
            const std::string& wantedExceptionArg,
            std::function<void()> onSuccess,
            std::function<void(const joynr::interlanguagetest::TestInterface::
                                       MethodWithExtendedErrorEnumErrorEnum::Enum& errorEnum)>
                    onError);

    virtual void methodToFireBroadcastWithSinglePrimitiveParameter(
            std::function<void()> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void methodToFireBroadcastWithMultiplePrimitiveParameters(
            std::function<void()> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void methodToFireBroadcastWithSingleArrayParameter(
            std::function<void()> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void methodToFireBroadcastWithMultipleArrayParameters(
            std::function<void()> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void methodToFireBroadcastWithSingleEnumerationParameter(
            std::function<void()> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void methodToFireBroadcastWithMultipleEnumerationParameters(
            std::function<void()> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void methodToFireBroadcastWithSingleStructParameter(
            std::function<void()> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void methodToFireBroadcastWithMultipleStructParameters(
            std::function<void()> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void methodToFireBroadcastWithFiltering(
            const std::string& stringArg,
            std::function<void()> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException&)> onError);

    virtual void getAttributeWithException(
            std::function<void(const bool&)> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void setAttributeWithException(
            const bool& attributeWithException,
            std::function<void()> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void getAttributeInt8readonlyNoSubscriptions(
            std::function<void(const int8_t&)> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void getAttributeBooleanReadonly(
            std::function<void(const bool&)> onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void getAttributeExtendedEnumerationReadonly(
            std::function<void(const joynr::interlanguagetest::namedTypeCollection2::
                                       ExtendedEnumerationWithPartlyDefinedValues::Enum&)>
                    onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException& exception)>
                    onError);

    virtual void methodWithSingleMapParameters(
            const joynr::interlanguagetest::namedTypeCollection2::MapStringString& mapArg,
            std::function<void(
                    const joynr::interlanguagetest::namedTypeCollection2::MapStringString& mapOut)>
                    onSuccess,
            std::function<void(const joynr::exceptions::ProviderRuntimeException&)> onError);

private:
    // Disallow copy and assign
    IltProvider(const IltProvider&);
    void operator=(const IltProvider&);

    std::mutex mutex; // Providers need to be threadsafe

    ADD_LOGGER(IltProvider);
};

#endif
