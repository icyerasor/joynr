/*
 * #%L
 * %%
 * Copyright (C) 2011 - 2014 BMW Car IT GmbH
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
#include "GeocastBroadcastFilter.h"
#include <QtCore>

#include "joynr/JsonSerializer.h"

GeocastBroadcastFilter::GeocastBroadcastFilter()
{
}

bool GeocastBroadcastFilter::filter(
        const joynr::vehicle::RadioStation& discoveredStation,
        const joynr::vehicle::GeoPosition& geoPosition,
        const vehicle::RadioNewStationDiscoveredBroadcastFilterParameters& filterParameters)
{
    if (filterParameters.getPositionOfInterest().isNull() ||
        filterParameters.getRadiusOfInterestArea().isNull()) {
        // filter parameter not set, so we do no filtering
        return true;
    }

    joynr::vehicle::GeoPosition* positionOfInterest =
            JsonSerializer::deserialize<joynr::vehicle::GeoPosition>(
                    filterParameters.getPositionOfInterest().toLatin1());
    int radiusOfInteresArea = filterParameters.getRadiusOfInterestArea().toInt();

    // calculate distance between two geo positions using the haversine formula
    // (cf. http://en.wikipedia.org/wiki/Haversine_formula)
    int earthRadius = 6371000; // in meters
    double lat1 = qDegreesToRadians(geoPosition.getLatitude());
    double lat2 = qDegreesToRadians(positionOfInterest->getLatitude());
    double long1 = qDegreesToRadians(geoPosition.getLongitude());
    double long2 = qDegreesToRadians(positionOfInterest->getLongitude());

    double latSinePow = qPow(qSin((lat2 - lat1) / 2), 2.0);
    double longSinePow = qPow(qSin((long2 - long1) / 2), 2.0);
    double help = qSqrt(latSinePow + qCos(lat1) * qCos(lat2) * longSinePow);
    // check for floating point errors
    if (help > 1.0) {
        help = 1.0;
    }
    double distance = 2 * earthRadius * qAsin(help);

    delete positionOfInterest;
    return distance < radiusOfInteresArea;
}