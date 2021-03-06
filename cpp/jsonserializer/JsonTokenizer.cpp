/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2015 BMW Car IT GmbH
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

#include "joynr/JsonTokenizer.h"

#include <utility>
#include <cassert>
#include <cctype>

namespace joynr
{

// Register types used in variants
static const bool isJsonArrayRegistered = Variant::registerType<JsonArray>("JsonArray");
static const bool isJsonObjectRegistered = Variant::registerType<JsonObject>("JsonObject");
static const bool isJsonValueRegistered = Variant::registerType<JsonValue>("JsonValue");

//--------- JsonToken ---------------------------------------------------------

JsonToken::JsonToken(const char *pStr, jsmntok_t jsmnToken) :
    type(jsmnToken.type),
    start(pStr + jsmnToken.start),
    end(pStr + jsmnToken.end),
    size(jsmnToken.size)
{
}

jsmntype_t JsonToken::getType() const
{
    return type;
}

std::string JsonToken::asString() const
{
    return std::string(start, end);
}

std::size_t JsonToken::getSize() const
{
    return size;
}

//--------- JsonObject --------------------------------------------------------

JsonObject::JsonObject(JsonTokenizer &tokenizer) :
    tokenizer(tokenizer),
    size(tokenizer.currentToken().getSize()),
    iterator(0),
    currentField()
{
}

bool JsonObject::hasNextField() const
{
    return iterator < size;
}

IField &JsonObject::nextField()
{
    tokenizer.nextToken();
    iterator += 1;
    currentField = std::make_unique<JsonField>(tokenizer);
    return *currentField;
}

//--------- JsonArray ---------------------------------------------------------

JsonArray::JsonArray(JsonTokenizer &tokenizer) :
    tokenizer(tokenizer),
    size(tokenizer.currentToken().getSize()),
    iterator(0),
    currentValue()
{
}

bool JsonArray::hasNextValue() const
{
    return iterator < size;
}

IValue &JsonArray::nextValue()
{
    tokenizer.nextToken();
    iterator += 1;
    currentValue = std::make_unique<JsonValue>(tokenizer);
    return *currentValue;
}

//--------- JsonValue ---------------------------------------------------------

JsonValue::JsonValue(JsonTokenizer &tokenizer) :
    value(Variant::NULL_VARIANT()),
    tokenizer(tokenizer)
{
    jsmntype_t tokenType = tokenizer.currentToken().getType();

    switch(tokenType) {
    case JSMN_ARRAY:
        value = Variant::make<JsonArray>(tokenizer);
        break;
    case JSMN_OBJECT:
        value = Variant::make<JsonObject>(tokenizer);
        break;
    case JSMN_PRIMITIVE:
        value = parseJsonPrimitive(tokenizer.currentToken().asString());
        break;
    case JSMN_STRING:
        value = Variant::make<std::string>(tokenizer.currentToken().asString());
        break;
    case JSMN_UNDEFINED:
    default:
        // Unknown type
        break;
    }
}

bool JsonValue::getBool() const
{
    if (!value.is<bool>()) {
        throw std::invalid_argument("Extracting int from non-string JsonValue");
    }
    return value.get<bool>();
}

JsonValue::operator const std::string&() const
{
    if (!value.is<std::string>()) {
        throw std::invalid_argument("Extracting string from non-string JsonValue");
    }
    return value.get<std::string>();
}

std::int64_t JsonValue::getInt64() const
{
    if (value.is<std::int64_t>()) {
        return value.get<std::int64_t>();
    } else if (value.is<std::uint64_t>()) {
        return static_cast<std::int64_t>(value.get<std::uint64_t>());
    } else {
        throw std::invalid_argument("Extracting std::int64_t from non-int JsonValue");
    }
}

double JsonValue::getDouble() const
{

    if (value.is<double>()) {
        return value.get<double>();
    } else if (value.is<std::int64_t>()) {
        return static_cast<double>(value.get<std::int64_t>());
    } else if (value.is<std::uint64_t>()) {
        return static_cast<double>(value.get<std::uint64_t>());
    }
    throw std::invalid_argument("Extracting double from non-double JsonValue");
}

std::uint64_t JsonValue::getUInt64() const
{
    if (!value.is<std::uint64_t>()) {
        throw std::invalid_argument("Extracting uint from non-uint JsonValue");
    }
    return value.get<std::uint64_t>();
}

Variant JsonValue::parseJsonPrimitive(const std::string &tokenString)
{
    // Is this true or false?
    if (tokenString == "true") {
        return Variant::make<bool>(true);
    } else if (tokenString == "false") {
        return Variant::make<bool>(false);
    }

    // Should the number be treated as an int or a float?
    // Assume a uint and see if any characters contradict this
    enum class NumberType { UINT, INT, FLOAT };
    NumberType numberType = NumberType::UINT;

    bool isFirstChar = true;

    for (char ch : tokenString) {
        // The first character can be a digit or a minus
        if (isFirstChar) {
            isFirstChar = false;
            if (ch == '-') {
                numberType = NumberType::INT;
            } else if (!std::isdigit(ch)) {
                // This is not a number - return an empty variant
                return Variant::NULL_VARIANT();
            }
        } else if (!std::isdigit(ch))  {
            // Any non-digits imply floating point
            if (ch == '.' || ch == 'e') {
                numberType = NumberType::FLOAT;
            } else {
                // This is not a number - return an empty variant
                return Variant::NULL_VARIANT();
            }
        }
    }

    // Convert based on the type of number
    if (numberType == NumberType::INT) {
        std::int64_t number = std::stoll(tokenString);
        return Variant::make<std::int64_t>(number);
    } else if (numberType == NumberType::UINT){
        std::uint64_t number = std::stoull(tokenString);
        return Variant::make<std::uint64_t>(number);
    } else {
        return Variant::make<double>(tokenizer.stringToDoubleLocaleIndependent(tokenString));
    }
}

JsonValue::operator IArray&()
{
    if (!value.is<JsonArray>()) {
        throw std::invalid_argument("Extracting array from non-array JsonValue");
    }
    return value.get<JsonArray>();
}

JsonValue::operator IObject&()
{
    if (!value.is<JsonObject>()) {
        throw std::invalid_argument("Extracting object from non-object JsonValue");
    }
    return value.get<JsonObject>();
}

bool JsonValue::isArray() const
{
    return value.is<JsonArray>();
}

bool JsonValue::isString() const
{
    return value.getTypeName() == "String";
}

bool JsonValue::isObject() const
{
    return value.is<JsonObject>();
}

Variant JsonValue::getVariant() const
{
    return value;
}

//--------- JsonField ---------------------------------------------------------

JsonField::JsonField(JsonTokenizer &tokenizer) :
    tokenizer(tokenizer),
    tokenKey(),
    tokenValue()
{
    tokenKey = std::make_unique<JsonValue>(this->tokenizer);
    this->tokenizer.nextToken();
    tokenValue = std::make_unique<JsonValue>(this->tokenizer);
}

const std::string &JsonField::name() const
{
    return key();
}

const IValue &JsonField::key() const
{
    return *tokenKey;
}

IValue &JsonField::value()
{
    return *tokenValue;
}

//--------- JsonTokenizer -----------------------------------------------------

std::atomic<std::size_t> JsonTokenizer::maxTokens{256};

JsonTokenizer::JsonTokenizer(const std::string &json) :
    source(json),
    tokens(maxTokens),
    currentIndex(0),
    parser(),
    valid(false),
    currentObject(),
    currentValue(),
    classicLocaleStream()
{
    jsmn_init(&parser);

    // Parse the JSON - valid remains false if any error occurs
    int res;
    while (true) {
        res = jsmn_parse(&parser, source.c_str(), json.size(),
                         tokens.data(), tokens.capacity());

        // Check for success
        if (res >= 0) {
            valid = true;
            break;
        }

        // If the token array is too small this application is parsing
        // unusually big JSON objects
        if (res == JSMN_ERROR_NOMEM) {
            std::size_t expected = tokens.size();
            std::size_t desired = expected * 2;
            std::atomic_compare_exchange_strong(&maxTokens, &expected, desired);
            tokens.resize(desired);
        } else {
            // A permanent error occured
            break;
        }
    }
}

bool JsonTokenizer::isValid() const
{
    return valid;
}

bool JsonTokenizer::hasNextObject() const
{
    // TODO: check the JSON and see how jsmn handles
    //       multiple objects
    return true;
}

IObject &JsonTokenizer::nextObject()
{
    assert(hasNextObject());
    currentObject = std::make_unique<JsonObject>(*this);
    return *currentObject;
}

bool JsonTokenizer::hasNextValue() const
{
    // TODO: check the JSON and see how jsmn handles
    //       multiple objects
    return true;
}

IValue &JsonTokenizer::nextValue()
{
    assert(hasNextValue());
    currentValue = std::make_unique<JsonValue>(*this);
    return *currentValue;
}

double JsonTokenizer::stringToDoubleLocaleIndependent(const std::string& doubleStr)
{
    classicLocaleStream << doubleStr;
    double doubleValue;
    classicLocaleStream >> doubleValue;
    classicLocaleStream.clear();
    return doubleValue;
}

// TODO: optimise this so the token is not recreated every time
//       have invalid JsonToken type
JsonToken JsonTokenizer::currentToken()
{
    return JsonToken(source.c_str(), tokens[currentIndex]);
}


JsonToken JsonTokenizer::nextToken()
{
    if (currentIndex >= tokens.size()) {
        throw std::out_of_range("No more Json tokens");
    }

    return JsonToken(source.c_str(), tokens[++currentIndex]);
}

} // namespace joynr

