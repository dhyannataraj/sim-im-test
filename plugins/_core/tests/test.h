/*
 * test.h
 *
 *  Created on: Aug 31, 2011
 */

#ifndef TEST_H_
#define TEST_H_


#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "tests/mocks/mockimagestorage.h"

testing::NiceMock<MockObjects::MockImageStorage>* getMockImageStorage();

#endif /* TEST_H_ */
