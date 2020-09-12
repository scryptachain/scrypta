/**
 * Copyright (c) 2015-2018 COLX Developers
 * Copyright (c) 2018-2019 Galilel Developers
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "context.h"
#include "sync.h"
#include "timedata.h"
#include "bootstrap/bootstrapmodel.h"

#include <memory>
#include <stdexcept>

static std::unique_ptr<CContext> context_;
static CCriticalSection csUpdate_;

void CreateContext()
{
    if (context_) {
        throw std::runtime_error("context has already been initialized, revise your code");
    } else {
        context_.reset(new CContext);
    }
}

void ReleaseContext()
{
    context_.reset();
}

CContext& GetContext()
{
    if (!context_) {
        throw std::runtime_error("context is not initialized");
    } else {
        return *context_;
    }
}

CContext::CContext()
{
    nStartupTime_ = GetAdjustedTime();
}

CContext::~CContext() {}

BootstrapModelPtr CContext::GetBootstrapModel()
{
    if (!bootstrapModel_) {
        bootstrapModel_.reset(new BootstrapModel);
    }

    return bootstrapModel_;
}
