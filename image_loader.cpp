/*
The MIT License (MIT)

Copyright (c) 2024 sigma-axis

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <cstdint>
#include <memory>

#define NOMINMAX
#include <Windows.h>

#include <gdiplus.h>
#pragma comment(lib, "gdiplus")

#include "image_loader.hpp"

namespace lib = sigma_lib::W32::GDI::plus;
namespace gp = Gdiplus;

static const gp::GdiplusStartupInput g_input{};
lib::image_loader::image_loader() noexcept
{
	gp::GdiplusStartup(&token, &g_input, nullptr);
}

lib::image_loader::~image_loader() noexcept
{
	if (token != 0) {
		gp::GdiplusShutdown(token);
		token = 0;
	}
}

lib::image_loader::result lib::image_loader::load(wchar_t const* path) const
{
	constexpr result invalid{ nullptr, 0, 0 };

	std::unique_ptr<gp::Bitmap> file{ gp::Bitmap::FromFile(path) };
	if (file->GetLastStatus() != Gdiplus::Ok) return invalid;

	int width = file->GetWidth(), height = file->GetHeight();
	if (width <= 0 || height <= 0) return invalid;

	if (file->GetPixelFormat() != PixelFormat32bppPARGB) {
		file.reset(file->Clone(0, 0, width, height, PixelFormat32bppPARGB));
		if (file->GetLastStatus() != Gdiplus::Ok) return invalid;
	}

	HBITMAP bmp;
	if (file->GetHBITMAP({ 0 }, &bmp) != Gdiplus::Ok) return invalid;

	return { bmp, width, height };
}

