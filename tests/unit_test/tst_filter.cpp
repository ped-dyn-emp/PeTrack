#include "filter.h"

#include <catch2/catch.hpp>


struct MyTestStruct
{
    int  state = 0;
    bool operator==(const MyTestStruct &rhs) const { return state == rhs.state; }
    bool operator!=(const MyTestStruct &rhs) const { return state != rhs.state; }
};

class SimpleFilter : public Filter
{
    // Filter interface
private:
    cv::Mat act(cv::Mat &img, cv::Mat &) override { return img; }
};


SCENARIO("I use a Parameter-Object to save a parameter")
{
    SimpleFilter filter;
    GIVEN("I create a double parameter object")
    {
        Parameter<double> testParam{&filter};
        THEN("The Parameter is initalized with zero")
        {
            CHECK(testParam.getValue() == 0.0);
            CHECK(testParam.getMinimum() == 0.0);
            CHECK(testParam.getMaximum() == 0.0);
        }

        AND_GIVEN("I set it to a value")
        {
            CHECK(!filter.changed());
            testParam.setValue(20);
            CHECK(testParam.getValue() == 20.0);
            THEN("The filter is notified")
            {
                CHECK(filter.changed());
            }
        }
    }
    GIVEN("I create an int parameter object")
    {
        Parameter<int> testParam{&filter};
        THEN("The Parameter is initalized with zero")
        {
            CHECK(testParam.getValue() == 0);
            CHECK(testParam.getMinimum() == 0);
            CHECK(testParam.getMaximum() == 0);
        }

        AND_GIVEN("I set it to a value")
        {
            CHECK(!filter.changed());
            testParam.setValue(20);
            CHECK(testParam.getValue() == 20);
            THEN("The filter is notified")
            {
                CHECK(filter.changed());
            }
        }
    }
    GIVEN("I create an bool parameter object")
    {
        Parameter<bool> testParam{&filter};
        THEN("The Parameter is initalized with false")
        {
            CHECK(testParam.getValue() == false);
            CHECK(testParam.getMinimum() == false);
            CHECK(testParam.getMaximum() == false);
        }

        AND_GIVEN("I set it to a value")
        {
            CHECK(!filter.changed());
            testParam.setValue(true);
            CHECK(testParam.getValue() == true);
            THEN("The filter is notified")
            {
                CHECK(filter.changed());
            }
        }
    }
    GIVEN("I create a struct parameter object")
    {
        Parameter<MyTestStruct> testParam{&filter};
        THEN("The Parameter is initalized with the default initialized struct")
        {
            CHECK(testParam.getValue() == MyTestStruct{});
            CHECK(testParam.getMinimum() == MyTestStruct{});
            CHECK(testParam.getMaximum() == MyTestStruct{});
        }

        AND_GIVEN("I set it to a value")
        {
            CHECK(!filter.changed());
            testParam.setValue(MyTestStruct{10});
            CHECK(testParam.getValue() == MyTestStruct{10});
            THEN("The filter is notified")
            {
                CHECK(filter.changed());
            }
        }
    }
}
