//
//  The MIT License (MIT)
//
//  Copyright (c) 2016 Tag Games Limited
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//

#ifndef _CHILLISOURCE_RENDERING_LIGHTING_RENDERAMBIENTLIGHT_H_
#define _CHILLISOURCE_RENDERING_LIGHTING_RENDERAMBIENTLIGHT_H_

#include <ChilliSource/ChilliSource.h>
#include <ChilliSource/Core/Base/Colour.h>

namespace ChilliSource
{
    /// A standard-layout container for data the renderer needs which pertains to a single
    /// ambient light, such as the colour.
    class RenderAmbientLight final
    {
    public:
        /// Creates a new instance of the container with the given light colour.
        ///
        /// @param colour
        ///     The colour of the light.
        ///
        RenderAmbientLight(const Colour& colour) noexcept;
        
        /// @return The colour of the light.
        ///
        const Colour& GetColour() const noexcept { return m_colour; }
        
    private:
        Colour m_colour;
    };
}

#endif
