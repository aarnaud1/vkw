/*
 * Copyright (c) 2025 Adrien ARNAUD
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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