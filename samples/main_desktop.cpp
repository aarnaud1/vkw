/*
 * Copyright (C) 2025 Adrien ARNAUD
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "RayQueryTriangle.hpp"
#include "SimpleTriangle.hpp"

#include <cstdlib>
#include <memory>

static std::unique_ptr<IGraphicsSample> graphicsSample = nullptr;

enum class TestCase : uint32_t
{
    SimpleTriangle = 0,
    RayQueryTriangle = 1,
    TestCaseCount = 2
};

int main(int argc, char** argv)
{
    if(argc <= 1)
    {
        fprintf(stderr, "Not enough arguments");
        return EXIT_FAILURE;
    }

    const uint32_t testCase = atoi(argv[1]);
    if(testCase >= static_cast<uint32_t>(TestCase::TestCaseCount))
    {
        fprintf(stderr, "Ibvalid test case");
        return EXIT_FAILURE;
    }

    try
    {
        switch(static_cast<TestCase>(testCase))
        {
            case TestCase::SimpleTriangle:
                graphicsSample.reset(new SimpleTriangle());
                break;
            case TestCase::RayQueryTriangle:
                graphicsSample.reset(new RayQueryTriangle());
                break;
            default:
                break;
        }

        if(graphicsSample != nullptr)
        {
            VKW_CHECK_BOOL_FAIL(graphicsSample->initSample(), "Error initializing sample");
            VKW_CHECK_BOOL_FAIL(graphicsSample->runSample(), "Error running sample");
        }
    }
    catch(std::exception& e)
    {
        fprintf(stderr, "%s\n", e.what());
        return EXIT_FAILURE;
    }

    graphicsSample.reset();
    return EXIT_SUCCESS;
}