/*
The MIT License (MIT)

Copyright (c) 2024 sigma-axis

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <cstdint>
#include <algorithm>
#include <bit>
#include <concepts>
#include <memory>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using byte = uint8_t;
#include <exedit.hpp>

#include "memory_protect.hpp"
#include "color_abgr.hpp"
using Color = sigma_lib::W32::GDI::Color;

#include "image_loader.hpp"
using image_loader = sigma_lib::W32::GDI::plus::image_loader;


////////////////////////////////
// 算術補助．
////////////////////////////////
namespace arith
{
	// division that rounds toward negative infinity.
	// `divisor` is assumed to be positive.
	template<std::signed_integral NumT>
	static constexpr auto floor_div(NumT dividend, NumT divisor) {
		if (dividend < 0) dividend -= divisor - 1;
		return dividend / divisor;
	}

	// calculates non-negative remainder.
	// `divisor` is assumed to be positive.
	template<std::signed_integral NumT>
	static constexpr auto normal_rem(NumT dividend, NumT divisor) {
		auto rem = dividend % divisor;
		if (rem < 0) rem += divisor;
		return rem;
	}
}


////////////////////////////////
// 主要情報源の変数アドレス．
////////////////////////////////
inline constinit struct ExEdit092 {
	AviUtl::FilterPlugin* fp;
	constexpr static auto info_exedit092 = "拡張編集(exedit) version 0.92 by ＫＥＮくん";
	bool init(AviUtl::FilterPlugin* this_fp)
	{
		if (fp != nullptr) return true;
		AviUtl::SysInfo si; this_fp->exfunc->get_sys_info(nullptr, &si);
		for (int i = 0; i < si.filter_n; i++) {
			auto that_fp = this_fp->exfunc->get_filterp(i);
			if (that_fp->information != nullptr &&
				0 == std::strcmp(that_fp->information, info_exedit092)) {
				fp = that_fp;
				init_pointers();
				return true;
			}
		}
		return false;
	}

	int32_t*	SettingDialogObjectIndex;	// 0x177a10
	ExEdit::Object**	ObjectArray_ptr;	// 0x1e0fa4
	int32_t*	NextObjectIdxArray;			// 0x1592d8

	int32_t*	timeline_h_scroll_pos;		// 0x1a52f0
	int32_t*	timeline_v_scroll_pos;		// 0x1a5308

	int32_t*	curr_timeline_scale_len;	// 0x0a3fc8
	int32_t*	curr_timeline_layer_height;	// 0x0a3e20

	int32_t*	timeline_width;				// 0x1a52fc
	int32_t*	timeline_height;			// 0x1a5300

	// font for the display text of each object.
	HFONT**		load_title_font;			// 0x037240 + 2
	byte*		call_select_obj_font;		// 0x037248

	// Ref: 蛇色様の Common-Library より当該関数のアドレス．
	// https://github.com/hebiiro/Common-Library/blob/62e8d04319ab6934d6aec6028f95a8ae69a05355/Common/AviUtlInternal.h#L103
	// m_DrawObject = (Type_DrawObject)(m_exedit + 0x00037060);
	void (*draw_object)(HDC, int32_t);		// 0x037060
	// callers of void draw_object(HDC, int32_t)
	byte*		call_draw_object[4];		// 0x037b53, 0x0393b6, 0x03945b, 0x039505

	// callers of void fill_grad(HDC dc, RECT const* rc, int32_t r1, int32_t g1, int32_t b1, int32_t r2, int32_t g2, int32_t b2, int32_t g_begin, int32_t g_end)
	// Ref: patch.aul のグラデーション描画関数のフック元．
	// https://github.com/nazonoSAUNA/patch.aul/blob/3675923d24ca9a62369dde08fb99ba92dfa97dfe/patch/patch_fast_exeditwindow.hpp#L65-L67
	byte*		call_fill_grad[3];		// 0x0374fa, 0x037562, 0x0375ba

	// Ref: patch.aul の選択中オブジェクトの点線の色を変える機能．
	// https://github.com/nazonoSAUNA/patch.aul/blob/9f7aa96871b29556e228a847356be9ee53acde4e/patch/patch_theme_cc.hpp#L461
	// OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x375fb, 4).store_i32(0, &disp_dialog_pen);
	HPEN**		SelectedObjectBorderPen;	// 0x0375fb

	// Ref: 蛇色様の OptimizeEditBox.auf より，当該関数，変数のアドレス．
	// https://github.com/hebiiro/AviUtl-Plugin-OptimizeEditBox/blob/56d6a5def3502f0f1bcb7b4f874dc1027adca89f/OptimizeEditBox/OptimizeEditBox.cpp#L60-L68
	byte*		DrawLayerBorderL;			// 0x038845
	byte*		DrawLayerBorderT;			// 0x038871
	byte*		DrawLayerBorderR;			// 0x0388aa
	byte*		DrawLayerBorderB;			// 0x0388da
	byte*		DrawLayerBorderSp;			// 0x037a1f

	Color**		color_time_sel_tip;			// 0x038076
	Color**		color_time_sel_int;			// 0x03807e
	Color**		color_time_sel_ext;			// 0x038087

private:
	void init_pointers()
	{
		auto pick_addr = [exedit_base = reinterpret_cast<uintptr_t>(fp->dll_hinst)]
			<class T>(T& target, ptrdiff_t offset) { target = reinterpret_cast<T>(exedit_base + offset); };
		pick_addr(SettingDialogObjectIndex,	0x177a10);
		pick_addr(ObjectArray_ptr,			0x1e0fa4);
		pick_addr(NextObjectIdxArray,		0x1592d8);

		pick_addr(timeline_h_scroll_pos,	0x1a52f0);
		pick_addr(timeline_v_scroll_pos,	0x1a5308);

		pick_addr(curr_timeline_scale_len,	0x0a3fc8);
		pick_addr(curr_timeline_layer_height,	0x0a3e20);

		pick_addr(timeline_width,			0x1a52fc);
		pick_addr(timeline_height,			0x1a5300);

		pick_addr(load_title_font,			0x037240 + 2);
		pick_addr(call_select_obj_font,		0x037248);

		pick_addr(draw_object,				0x037060);
		pick_addr(call_draw_object[0],		0x037b53);
		pick_addr(call_draw_object[1],		0x0393b6);
		pick_addr(call_draw_object[2],		0x03945b);
		pick_addr(call_draw_object[3],		0x039505);

		pick_addr(call_fill_grad[0],		0x0374fa);
		pick_addr(call_fill_grad[1],		0x037562);
		pick_addr(call_fill_grad[2],		0x0375ba);

		pick_addr(SelectedObjectBorderPen,	0x0375fb);

		pick_addr(DrawLayerBorderL,			0x038845);
		pick_addr(DrawLayerBorderT,			0x038871);
		pick_addr(DrawLayerBorderR,			0x0388aa);
		pick_addr(DrawLayerBorderB,			0x0388da);
		pick_addr(DrawLayerBorderSp,		0x037a1f);

		pick_addr(color_time_sel_tip,		0x038076);
		pick_addr(color_time_sel_int,		0x03807e);
		pick_addr(color_time_sel_ext,		0x038087);
	}
} exedit;


////////////////////////////////
// 座標変換．
////////////////////////////////
static constexpr struct {
	// タイムラインウィンドウの座標と，フレーム数 / レイヤー数との間の変換．

	// タイムラインの最上段レイヤーの左上座標．
	constexpr static int x_leftmost_client = 64, y_topmost_client = 42;

	// 内部管理のピクセルサイズの分母．
	constexpr static int scale_denom = 10'000;

	// x 座標 → フレーム数．
	static inline int PointToFrame(int x)
	{
		x -= x_leftmost_client;
		x *= scale_denom;
		x = arith::floor_div(x, *exedit.curr_timeline_scale_len);
		x += *exedit.timeline_h_scroll_pos;
		if (x < 0) x = 0;

		return x;
	}
	// フレーム数 → x 座標．
	static inline int FrameToPoint(int f)
	{
		f -= *exedit.timeline_h_scroll_pos;
		{
			// possibly overflow; curr_timeline_scale_len is at most 100'000 (> 2^16).
			int64_t F = f;
			F *= *exedit.curr_timeline_scale_len;
			F = arith::floor_div<int64_t>(F, scale_denom);
			f = static_cast<int32_t>(F);
		}
		f += x_leftmost_client;

		return f;
	}

	// y 座標 → レイヤー数．
	static int PointToLayer(int y)
	{
		// 最上段レイヤーは 0．
		y -= y_topmost_client;
		y = arith::floor_div(y, *exedit.curr_timeline_layer_height);
		y += *exedit.timeline_v_scroll_pos;
		y = std::clamp(y, 0, 99);

		return y;
	}
	// レイヤー数 → y 座標．
	static int LayerToPoint(int l)
	{
		l -= *exedit.timeline_v_scroll_pos;
		l *= *exedit.curr_timeline_layer_height;
		l += y_topmost_client;

		return l;
	}
} timeline_coord;


////////////////////////////////
// 枠線定義．
////////////////////////////////
union thickness {
	// 枠線の太さを記述する．WPF の Thickness 構造体の類似物．
	uint32_t raw;
	struct { int8_t left, top, right, bottom; };

	constexpr thickness() : raw{ 0 } {}
	constexpr thickness(uint32_t raw) : raw{ raw } {}
	constexpr thickness(int8_t left, int8_t top, int8_t right, int8_t bottom) :
		left{ left }, top{ top }, right{ right }, bottom{ bottom } {}

	constexpr thickness& operator+=(thickness const& t2) {
		left += t2.left;
		top += t2.top;
		right += t2.right;
		bottom += t2.bottom;
		return *this;
	}
	constexpr friend thickness operator+(thickness t1, thickness const& t2) {
		t1 += t2;
		return t1;
	}
	constexpr bool is_non_negative() const { return (raw & 0x80808080) == 0; }
	constexpr bool is_zero() const { return raw == 0; }
	constexpr bool is_positive() const { return !is_zero() && is_non_negative(); }
	constexpr bool is_uniform_one() const { return raw == 0x01010101; }

	// 文字列からの立ち上げ．
	static bool parse(char const* str, thickness& result)
	{
		int a, b, c, d;
		int const n = ::sscanf_s(str, "%i,%i,%i,%i", &a, &b, &c, &d);

		constexpr auto to_byte = [](int x) -> int8_t { return std::clamp(x, -128, 127); };
		int8_t const A = to_byte(a), B = to_byte(b), C = to_byte(c), D = to_byte(d);
		switch (n) {
		case 1: result = { A, A, A, A }; return true;
		case 2: result = { A, B, A, B }; return true;
		case 4: result = { A, B, C, D }; return true;
		}
		return false;
	}

	// RECT を太さ分だけ縮小する．
	constexpr void deflate_rect(RECT& rc) const
	{
		rc.left += left; rc.right -= right;
		rc.top += top; rc.bottom -= bottom;
	}
};
struct colored_frame {
	// 色付き枠線．描画位置も外側の距離として記述できる．
	Color color;
	thickness margin, thick;
	constexpr void normalize()
	{
		if (color == CLR_INVALID || !thick.is_positive()) {
			color.raw = CLR_INVALID;
			margin += thick;
			thick = 0;
		}
		else color = color.remove_alpha();
	}
	inline void draw(HDC hdc, RECT& rc) const;

	constexpr bool operator==(colored_frame const& other) const
	{
		return color == other.color &&
			margin.raw == other.margin.raw &&
			thick.raw == other.thick.raw;
	}
	constexpr bool is_effective() const { return thick.is_positive(); }
};
struct obj_border {
	// 2重枠線としてオブジェクトに描画．
	colored_frame outer, inner;
	constexpr void normalize()
	{
		outer.normalize(); inner.normalize();
		if (!outer.is_effective()) {
			inner.margin += outer.margin;
			outer = inner;
			inner = {};

		}
		if (!outer.is_effective()) outer = {};
		if (!inner.is_effective()) inner = {};
	}
	void draw(HDC hdc, RECT const& rc) const
	{
		if (outer.is_effective()) {
			RECT rc2 = rc;
			outer.draw(hdc, rc2);
			if (inner.is_effective())
				inner.draw(hdc, rc2);
		}
	}

	constexpr bool operator==(obj_border const& other) const
	{
		return outer == other.outer && inner == other.inner;
	}
	constexpr bool is_effective() const { return outer.is_effective(); }
};


////////////////////////////////
// 設定項目．
////////////////////////////////
inline constinit struct Settings {
	enum class grad_type : uint8_t {
		slope = 0,
		steps = 1,
	};
	struct {
		grad_type type = grad_type::slope;
		int steps = 4;
		constexpr static int min_steps = 1, max_steps = 256;
	} gradient_back;

	obj_border non_selected_border{
		.outer{.color{}, .margin{}, .thick{} },
		.inner{.color{}, .margin{}, .thick{} },
	}, selected_border{
		.outer{.color{ 0xffffff }, .margin{ 1, 1, 1, 1 }, .thick{ 1, 1, 1, 1 } },
		.inner{.color{}, .margin{}, .thick{} },
	}; 

	bool hide_dot_border = true;

	enum class image_align : int8_t {
		scroll_lo = 0,
		client_lo = 1,
		client_md = 2,
		client_hi = 3,
		object_lo = 4,
		object_md = 5,
		object_hi = 6,
	};
	struct overlay {
		std::unique_ptr<wchar_t[]> path{};
		byte alpha = 0; bool connect_midpoint = true;
		constexpr static byte min_alpha = 0, max_alpha = 255;
		image_align align_h = image_align::scroll_lo, align_v = image_align::scroll_lo;
		thickness margin{};
		operator bool() const { return path && alpha > 0; }
	} overlay_selected, overlay_non_selected;

	// default font: name="Yu Gothic UI", size=-12, wt=400.
	HFONT title_font = nullptr, title_font_selected = nullptr;

	struct {
		Color left, top, right, bottom, separator;
	} layer_border_color{ CLR_INVALID, CLR_INVALID, CLR_INVALID, CLR_INVALID, CLR_INVALID };

	struct {
		Color tip, interior, exterior;
	} time_sel_color{ CLR_INVALID, CLR_INVALID, CLR_INVALID };

	void load(char const* inifile)
	{
		auto read_int = [=](char const* section, char const* key, int def) {
			return ::GetPrivateProfileIntA(section, key, def, inifile);
		};
		auto read_thick = [=](char const* section, char const* key, thickness def) {
			char buf[32];
			if (::GetPrivateProfileStringA(section, key, "", buf, std::size(buf), inifile) > 0)
				thickness::parse(buf, def);
			return def;
		};
		auto read_font = [=](char const* section) -> HFONT {
			constexpr int def_size = 0;

			char buf[3 * LF_FACESIZE];
			if (::GetPrivateProfileStringA(section, "name", "", buf, std::size(buf), inifile) <= 0)
				return nullptr;

			int size = ::GetPrivateProfileIntA(section, "size", def_size, inifile);
			if (size == 0) return nullptr;

			wchar_t name[LF_FACESIZE];
			if (::MultiByteToWideChar(CP_UTF8, 0, buf, -1, name, std::size(name)) <= 0)
				return nullptr;

			return ::CreateFontW(size, 0, 0, 0, FW_DONTCARE,
				FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, name);
		};
		auto read_path = [=](char const* section, char const* key) -> std::unique_ptr<wchar_t[]> {
			char buf[3 * MAX_PATH];
			if (::GetPrivateProfileStringA(section, key, "", buf, std::size(buf), inifile) <= 0)
				return nullptr;

			wchar_t wbuf[MAX_PATH];
			auto len = ::MultiByteToWideChar(CP_UTF8, 0, buf, -1, wbuf, std::size(wbuf));
			if (len <= 0) return nullptr;

			auto ret = std::make_unique_for_overwrite<wchar_t[]>(len);
			std::wmemcpy(ret.get(), wbuf, len);
			return ret;
		};

	#define load_gen(head, key, section, read, write)	head##key =	static_cast<decltype(head##key)>(\
		read##(read_int(section, #key, static_cast<int>(write##(head##key)))))
	#define load_int(head, key, section)				load_gen(head, key, section, \
		([m=head##min_##key,M=head##max_##key](UINT x) { return std::clamp(static_cast<decltype(m)>(x), m, M); }), /* id */)
	#define load_bool(head, key, section)				load_gen(head, key, section, 0 != , [](bool x) { return x ? 1 : 0; })
	#define load_enum(head, key, section)				load_gen(head, key, section, /* id */, /* id */)
	#define load_color(head, key, section)				load_gen(head, key, section, \
		Color::fromARGB, [](Color y) { return y.to_formattable(); })
	#define load_thick(head, key, section)	head##key = read_thick(section, #key, head##key)
	#define load_border(border, section)	\
		load_color(border., outer.color, section);	\
		load_thick(border., outer.margin, section);	\
		load_thick(border., outer.thick, section);	\
		load_color(border., inner.color, section);	\
		load_thick(border., inner.margin, section);	\
		load_thick(border., inner.thick, section);	\
		border.normalize()
	#define load_overlay(overlay, section)	\
		overlay.path = read_path(section, "path");	\
		load_int(overlay., alpha, section);			\
		load_enum(overlay., align_h, section);		\
		load_enum(overlay., align_v, section);		\
		load_thick(overlay., margin, section);		\
		load_bool(overlay., connect_midpoint, section)

		load_enum(gradient_back., type,		"gradient_back");
		load_int(gradient_back., steps,		"gradient_back");

		load_overlay(overlay_selected,		"overlay.selected");
		load_overlay(overlay_non_selected,	"overlay.deselected");

		load_border(non_selected_border,	"border.deselected");
		load_border(selected_border,		"border.selected");
		load_bool(, hide_dot_border,		"border.selected");

		title_font = read_font("title_font");
		title_font_selected = read_font("title_font.selected");

		load_color(layer_border_color., left,		"layer_border");
		load_color(layer_border_color., top,		"layer_border");
		load_color(layer_border_color., right,		"layer_border");
		load_color(layer_border_color., bottom,		"layer_border");
		load_color(layer_border_color., separator,	"layer_border");

		load_color(time_sel_color., tip,		"time_selection");
		load_color(time_sel_color., interior,	"time_selection");
		load_color(time_sel_color., exterior,	"time_selection");

	#undef load_overlay
	#undef load_border
	#undef load_thick
	#undef load_color
	#undef load_enum
	#undef load_bool
	#undef load_int
	#undef load_gen
	}
} settings;


////////////////////////////////
// 設定ロードセーブ．
////////////////////////////////

// replacing a file extension when it's known.
template<class T, size_t len_max, size_t len_old, size_t len_new>
void replace_tail(T(&dst)[len_max], size_t len, const T(&tail_old)[len_old], const T(&tail_new)[len_new])
{
	if (len < len_old || len - len_old + len_new > len_max) return;
	std::memcpy(dst + len - len_old, tail_new, len_new * sizeof(T));
}
inline void load_settings(HMODULE h_dll)
{
	char path[MAX_PATH];
	replace_tail(path, ::GetModuleFileNameA(h_dll, path, std::size(path)) + 1, "auf", "ini");

	settings.load(path);
}


////////////////////////////////
// 選択中オブジェクトの検知．
////////////////////////////////
inline constinit struct {
	int32_t drawingIndex = 0;
	bool is_selected() const { return drawingIndex == *exedit.SettingDialogObjectIndex; }

	static void draw_object_detour(HDC dc, int32_t object_index)
	{
		draw_object.drawingIndex = object_index;
		exedit.draw_object(dc, object_index);
	}
} draw_object;


////////////////////////////////
// 画像のオーバーレイ．
////////////////////////////////
static inline constinit struct image_overlay
{
	HDC mem_dc = nullptr;
	HGDIOBJ old_bmp = nullptr;
	int width = 0, height = 0, unit_width = 0, unit_height = 0;
	thickness margin{};
	byte alpha = 0; bool connect_midpoint = true;

	constexpr static int min_width = 256, min_height = 32;
	void load(Settings::overlay const& settings, HDC src_dc, image_loader const& loader)
	{
		dispose();

		if (!settings) return;
		auto const img = loader.load(settings.path.get());
		if (!img) return; // 画像の取得失敗．

		alpha = settings.alpha; margin = settings.margin;
		connect_midpoint = settings.connect_midpoint;

		unit_width = img.width; unit_height = img.height;
		if (unit_width >= min_width && unit_height >= min_height) {
			// タイル描画に十分な大きさなので，そのまま保存．
			mem_dc = ::CreateCompatibleDC(src_dc);
			old_bmp = ::SelectObject(mem_dc, img.bitmap);
			width = unit_width; height = unit_height;
		}
		else {
			// 効率的なタイル描画には小さすぎるので，予め大きめのサイズになるまで並べておく．
			int rep_x = (unit_width + min_width - 1) / unit_width,
				rep_y = (unit_height + min_height - 1) / unit_height;

			width = rep_x * unit_width; height = rep_y * unit_height;
			mem_dc = ::CreateCompatibleDC(src_dc);
			old_bmp = ::SelectObject(mem_dc, ::CreateBitmap(width, height, 1, 32, nullptr));

			auto temp_dc = ::CreateCompatibleDC(src_dc);
			auto temp_bmp = ::SelectObject(temp_dc, img.bitmap);
			for (int j = 0; j < rep_y; j++) {
				for (int i = 0; i < rep_x; i++)
					::BitBlt(mem_dc, i * unit_width, j * unit_height, unit_width, unit_height,
						temp_dc, 0, 0, SRCCOPY);
			}
			::DeleteObject(::SelectObject(temp_dc, temp_bmp));
			::DeleteDC(temp_dc);
		}

		// 画像配置の座標計算の関数ポインタを取得．
		set_origin_x(choose_origin_x(settings.align_h));
		set_origin_y(choose_origin_y(settings.align_v));
	}

	constexpr bool is_effective() const { return mem_dc != nullptr; }

	// リソース破棄．
	void dispose()
	{
		if (is_effective()) {
			::DeleteObject(::SelectObject(mem_dc, old_bmp));
			old_bmp = nullptr;

			::DeleteDC(mem_dc);
			mem_dc = nullptr;
		}
	}

	// 描画関数．
	void render(HDC dc, RECT const& rc) const
	{
		if (!is_effective()) return;

		RECT rc2 = rc;
		if (!margin.is_zero()) {
			margin.deflate_rect(rc2);
			if (connect_midpoint && (margin.left != 0 || margin.right != 0)) {
				if (int head = (*exedit.ObjectArray_ptr)[draw_object.drawingIndex].index_midpt_leader;
					head >= 0) {
					if (head != draw_object.drawingIndex) rc2.left = rc.left;
					if (exedit.NextObjectIdxArray[draw_object.drawingIndex] >= 0) rc2.right = rc.right;
				}
			}
			if (rc2.left >= rc2.right || rc2.top >= rc2.bottom) return;
		}


		BLENDFUNCTION const bfn{
			.BlendOp = AC_SRC_OVER,
			.BlendFlags = 0,
			.SourceConstantAlpha = alpha,
			.AlphaFormat = AC_SRC_ALPHA,
		};

		// ::GdiAlphaBlend() でタイル描画していく．
		int ix = arith::normal_rem<int>(rc2.left - (this->*get_origin_x())(), unit_width),
			iy = arith::normal_rem<int>(rc2.top - (this->*get_origin_y())(rc2.top, rc2.bottom), unit_height);
		for (int py = rc2.top; py < rc2.bottom; py += height - iy, iy = 0) {
			int h = std::min<int>(rc2.bottom - py, height - iy);
			for (int px = rc2.left, ix1 = ix; px < rc2.right; px += width - ix1, ix1 = 0) {
				int w = std::min<int>(rc2.right - px, width - ix1);
				::GdiAlphaBlend(dc, px, py, w, h, mem_dc, ix1, iy, w, h, bfn);
			}
		}
	}

private:
	// パターンの基準座標を計算．
	using func_origin_x = int (image_overlay::*)() const;
	using func_origin_y = int (image_overlay::*)(int dst_y0, int dst_y1) const;
	void* origin_x = nullptr; // use void* to avoid compile-time error ---
	void* origin_y = nullptr; // pointer-to-member function isn't recognized constexpr even if it's null.
	constexpr void set_origin_x(func_origin_x value) { origin_x = std::bit_cast<void*>(value); }
	constexpr func_origin_x get_origin_x() const { return std::bit_cast<func_origin_x>(origin_x); }
	constexpr void set_origin_y(func_origin_y value) { origin_y = std::bit_cast<void*>(value); }
	constexpr func_origin_y get_origin_y() const { return std::bit_cast<func_origin_y>(origin_y); }

	static constexpr func_origin_x choose_origin_x(Settings::image_align align_h)
	{
		using enum Settings::image_align;
		switch (align_h) {
		case scroll_lo:
		default:				return &origin_x_scroll_lo;
		case client_lo:	return &origin_x_client_lo;
		case client_md:	return &origin_x_client_md;
		case client_hi:	return &origin_x_client_hi;
		case object_lo:	return &origin_x_object_lo;
		case object_md:	return &origin_x_object_md;
		case object_hi:	return &origin_x_object_hi;
		}
	}

	static constexpr func_origin_y choose_origin_y(Settings::image_align align_v)
	{
		using enum Settings::image_align;
		switch (align_v) {
		case scroll_lo:
		default:				return &origin_y_scroll_lo;
		case client_lo:	return &origin_y_client_lo;
		case client_md:	return &origin_y_client_md;
		case client_hi:	return &origin_y_client_hi;
		case object_lo:	return &origin_y_object_lo;
		case object_md:	return &origin_y_object_md;
		case object_hi:	return &origin_y_object_hi;
		}
	}

	int origin_x_scroll_lo() const { return timeline_coord.FrameToPoint(0); }
	int origin_x_client_lo() const { return timeline_coord.x_leftmost_client; }
	int origin_x_client_md() const {
		return ((timeline_coord.x_leftmost_client + *exedit.timeline_width - unit_width) >> 1);
	}
	int origin_x_client_hi() const { return *exedit.timeline_width; }
	int origin_x_object_lo() const
	{
		auto const objects = *exedit.ObjectArray_ptr;
		auto* obj = &objects[draw_object.drawingIndex];
		if (obj->index_midpt_leader >= 0) obj = &objects[obj->index_midpt_leader];
		return timeline_coord.FrameToPoint(obj->frame_begin);
	}
	int origin_x_object_md() const
	{
		auto const [b, e] = chain_begin_end(draw_object.drawingIndex);
		return (timeline_coord.FrameToPoint(b->frame_begin)
			+ timeline_coord.FrameToPoint(e->frame_end + 1) - unit_width) >> 1;
	}
	int origin_x_object_hi() const
	{
		auto const [_, e] = chain_begin_end(draw_object.drawingIndex);
		return timeline_coord.FrameToPoint(e->frame_end + 1);
	}
	static std::pair<ExEdit::Object*, ExEdit::Object*> chain_begin_end(int idx)
	{
		auto objects = *exedit.ObjectArray_ptr;
		auto* obj = &objects[idx];
		if (obj->index_midpt_leader < 0) return { obj, obj };

		int i = draw_object.drawingIndex, j;
		while (j = exedit.NextObjectIdxArray[i], j >= 0) i = j;
		return { &objects[obj->index_midpt_leader], &objects[i] };
	}

	int origin_y_scroll_lo(int, int) const { return timeline_coord.LayerToPoint(0); }
	int origin_y_client_lo(int, int) const { return timeline_coord.y_topmost_client; }
	int origin_y_client_md(int, int) const {
		return ((timeline_coord.y_topmost_client + *exedit.timeline_height - unit_height) >> 1);
	}
	int origin_y_client_hi(int, int) const { return *exedit.timeline_height; }
	int origin_y_object_lo(int dst_y0, int) const { return dst_y0; }
	int origin_y_object_md(int dst_y0, int dst_y1) const {
		return (dst_y0 + dst_y1 - unit_height) >> 1;
	}
	int origin_y_object_hi(int, int dst_y1) const { return dst_y1; }

} overlay_non_selected, overlay_selected;


////////////////////////////////
// グラデーション描画．
////////////////////////////////
inline constinit struct
{
	HBRUSH dc_brush = nullptr;

	static void fill_rect(HDC dc, const RECT& rc, Color color)
	{
		::SetDCBrushColor(dc, color);
		::FillRect(dc, &rc, gradient_fill.dc_brush);
	}

	static void grad_fill(HDC dc, int top, int bottom, int left, int right,
		uint16_t r1, uint16_t g1, uint16_t b1, uint16_t r2, uint16_t g2, uint16_t b2)
	{
		constexpr GRADIENT_RECT rects[] = { {.UpperLeft = 0, .LowerRight = 1 } };
		TRIVERTEX verts[] = {
			{.x = left,  .y = top,    .Red = r1, .Green = g1, .Blue = b1 },
			{.x = right, .y = bottom, .Red = r2, .Green = g2, .Blue = b2 },
		};
		::GdiGradientFill(dc, verts, std::size(verts), const_cast<GRADIENT_RECT*>(rects), std::size(rects), GRADIENT_FILL_RECT_H);
	}

private:
	// 呼び出された場合，rc->left < rc->right が成り立っていて，仮に幅が 0 になるくらいの
	// 小さい拡大率と短いオブジェクトであっても最低幅 1 ピクセルが保証されている模様．

	// 単色塗りつぶし目的でもこれらの関数が呼ばれる (中間点の間や Ctrl 選択時など).
	// その場合 r1 = r2, g1 = g2, b1 = b2, g_begin = 0, g_end = 0 という状況になっている．
	// また，rc->left は最小でも 64.

	// グラデーション幅は，端をつまんでドラッグ中のオブジェクトは 20 ピクセル，
	// 上述の単色塗りつぶしの場合は 0, それ以外はオブジェクト幅となっている模様．
	// 単色塗りつぶしさえ処理しておけば 0 除算の心配はない．

	// グラデーションの描画 パターン 0. オリジナルの処理に近い．
	constexpr static auto gradation = [](HDC dc, RECT const* rc,
		int32_t r1, int32_t g1, int32_t b1, int32_t r2, int32_t g2, int32_t b2, int32_t g_begin, int32_t g_end)
	{
		// デフォルト処理に近いグラデーション。
		// patch.aul のコードを参考に書き直した．
		// 参考にしました:
		// https://github.com/ePi5131/patch.aul/blob/d46adbf95c87e52fe021222aea02837fdd03a6ef/patch/patch_fast_exeditwindow.cpp#L28
		// オーバーフロー対策は 64 bit に伸長するのではなく，予め下位ビットを切り落としておく方針に．

		if (g_begin == rc->left && g_end == rc->right) [[likely]]
			// こまごましたオブジェクトはほとんどが単純グラデーション．
			grad_fill(dc, rc->top, rc->bottom, g_begin, g_end,
				r1 << 8, g1 << 8, b1 << 8, r2 << 8, g2 << 8, b2 << 8);
		else if (g_end <= rc->left) fill_rect(dc, *rc, { r2, g2, b2 });
		else if (rc->right <= g_begin) fill_rect(dc, *rc, { r1, g1, b1 });
		else {
			r1 <<= 8; g1 <<= 8; b1 <<= 8;
			r2 <<= 8; g2 <<= 8; b2 <<= 8;

			// denominate lower bits to prevent overflows in later calculations.
			int c_len = g_end - g_begin,
				c_left = rc->left - g_begin, c_right = rc->right - g_begin;
			if (c_len >= (1 << 15)) [[unlikely]] {
				auto const l = std::bit_width(static_cast<uint32_t>(c_len)) - 15;
				c_len >>= l; c_left >>= l; c_right >>= l;

				// note: one frame renders at most 10-pixel-wide.
				// maximum number of frames for a scene is 2^23-1.
			}
			// hereafter, c_len, c_left and c_right are all < 2^15.

			int const dR = r2 - r1, dG = g2 - g1, dB = b2 - b1;
			if (rc->left < g_begin)
				fill_rect(dc, { rc->left, rc->top, g_begin, rc->bottom },
					(r1 >> 8) | g1 | (b1 << 8));
			else {
				r1 += dR * c_left / c_len;
				g1 += dG * c_left / c_len;
				b1 += dB * c_left / c_len;
				g_begin = rc->left;
			}

			if (g_end < rc->right)
				fill_rect(dc, { g_end, rc->top, rc->right, rc->bottom },
					(r2 >> 8) | g2 | (b2 << 8));
			else {
				c_right -= c_len;
				r2 += dR * c_right / c_len;
				g2 += dG * c_right / c_len;
				b2 += dB * c_right / c_len;
				g_end = rc->right;
			}

			grad_fill(dc, rc->top, rc->bottom, g_begin, g_end,
				r1, g1, b1, r2, g2, b2);
		}
	};


	// グラデーションの描画 パターン 1. 階段状のグラデーション．
	constexpr static auto steps = [](HDC dc, RECT const* rc,
		int32_t r1, int32_t g1, int32_t b1, int32_t r2, int32_t g2, int32_t b2, int32_t g_begin, int32_t g_end)
	{
		// 参考にしました:
		// https://github.com/ePi5131/patch.aul/blob/d46adbf95c87e52fe021222aea02837fdd03a6ef/patch/patch_fast_exeditwindow.cpp#L138
		// 中間点の境目を判別できるように，色の変化範囲は 0% -- 100% ではなく 0.5/N -- 1-0.5/N にしている．

		int const grad_steps = settings.gradient_back.steps; // >= 1.

		if (g_end <= rc->left) fill_rect(dc, *rc, { r2, g2, b2 });
		else if (rc->right <= g_begin) fill_rect(dc, *rc, { r1, g1, b1 });
		else {
			RECT step{ .top = rc->top, .right = rc->left, .bottom = rc->bottom };
			auto const [i_len, i_rem] = std::div(g_end - g_begin, grad_steps); // to avoid overflowing.
			Color const c1{ r1, g1, b1 }, c2{ r2, g2, b2 };
			for (int i = 0; step.right < rc->right; i++) {
				step.left = std::max(step.right, rc->left);

				if (i <= grad_steps)
					step.right = std::min<int32_t>(rc->right,
						g_begin + i_len * i + (i_rem * i) / grad_steps);
				else step.right = rc->right;

				if (step.left >= step.right) continue;

				int I = 2 * i - 1;
				fill_rect(dc, step, I < 0 ? c1 : I > 2 * grad_steps ? c2 :
					Color::interpolate(c1, c2, 2 * grad_steps - I, I));
			}
		}
	};

	template<auto const& grad, bool draw_overlay, bool draw_border, bool same_border>
	static void __cdecl grad_fill(HDC dc, RECT const* rc,
		int32_t r1, int32_t g1, int32_t b1, int32_t r2, int32_t g2, int32_t b2, int32_t g_begin, int32_t g_end)
	{
		grad(dc, rc, r1, g1, b1, r2, g2, b2, g_begin, g_end);

		// 背景画像と枠を描画．
		if constexpr (draw_overlay || draw_border) {
			if constexpr (draw_overlay || !same_border) {
				if (draw_object.is_selected()) {
					if constexpr (draw_overlay) overlay_selected.render(dc, *rc);
					if constexpr (draw_border) settings.selected_border.draw(dc, *rc);
				}
				else {
					if constexpr (draw_overlay) overlay_non_selected.render(dc, *rc);
					if constexpr (draw_border) settings.non_selected_border.draw(dc, *rc);
				}
			}
			else {
				settings.non_selected_border.draw(dc, *rc);
			}
		}
	}

	template<auto const& grad>
	static constexpr auto choose_grad_step1() {
		if (overlay_non_selected.is_effective() || overlay_selected.is_effective())
			return choose_grad_step2<grad, true>();
		return choose_grad_step2<grad, false>();
	}
	template<auto const& grad, bool draw_overlay>
	static constexpr auto choose_grad_step2() {
		if (settings.non_selected_border.is_effective() || settings.selected_border.is_effective()) {
			if (settings.non_selected_border == settings.selected_border)
				return &grad_fill<grad, draw_overlay, true, true>;
			return &grad_fill<grad, draw_overlay, true, false>;
		}
		return &grad_fill<grad, draw_overlay, false, true>;
	}

public:
	static constexpr auto choose_grad(Settings::grad_type type)
	{
		switch (type) {
			using gt = Settings::grad_type;
		case gt::steps:	return choose_grad_step1<steps>();
		case gt::slope:
		default:		return choose_grad_step1<gradation>();
		}
	}
} gradient_fill;

// 枠線．
inline void colored_frame::draw(HDC dc, RECT& rc) const
{
	margin.deflate_rect(rc);
	if (rc.left >= rc.right || rc.top >= rc.bottom) return;

	::SetDCBrushColor(dc, color);
	if (thick.is_uniform_one()) {
		::FrameRect(dc, &rc, gradient_fill.dc_brush);
		rc.left++; rc.right--;
		rc.top++; rc.bottom--;
	}
	else if (rc.right - rc.left <= thick.left + thick.right ||
		rc.bottom - rc.top <= thick.top + thick.bottom) {
		::FillRect(dc, &rc, gradient_fill.dc_brush);
		rc.left += thick.left; rc.right -= thick.right;
		rc.top += thick.top; rc.bottom -= thick.bottom;
	}
	else {
		auto v = rc.right, w = rc.right = rc.left + thick.left;
		if (thick.left > 0) ::FillRect(dc, &rc, gradient_fill.dc_brush);
		rc.right = v;
		rc.left = rc.right - thick.right;
		if (thick.right > 0) ::FillRect(dc, &rc, gradient_fill.dc_brush);

		rc.right = rc.left; rc.left = w;

		v = rc.bottom; w = rc.bottom = rc.top + thick.top;
		if (thick.top > 0) ::FillRect(dc, &rc, gradient_fill.dc_brush);
		rc.bottom = v;
		rc.top = rc.bottom - thick.bottom;
		if (thick.bottom > 0) ::FillRect(dc, &rc, gradient_fill.dc_brush);

		rc.bottom = rc.top; rc.top = w;
	}
}

// レイヤーを囲む矩形の縁色指定．
// ダークモード系プラグイン適用時には利用されない．
struct LayerBorder {
	// 高 DPI でも「ゴミ」が残らないよう，HPEN ではなく HBRUSH による描画を行う．
	static void __cdecl left(HDC dc, int32_t mx, int32_t my, int32_t lx, int32_t ly, HPEN pen) {
		// practically, almost hidden by the "separator" lines.
		gradient_fill.fill_rect(dc, { mx, my, lx + 1, ly + 1 }, settings.layer_border_color.left);
	}
	static void __cdecl top(HDC dc, int32_t mx, int32_t my, int32_t lx, int32_t ly, HPEN pen) {
		gradient_fill.fill_rect(dc, { mx, my, lx + 1, ly + 1 }, settings.layer_border_color.top);
	}
	static void __cdecl right(HDC dc, int32_t mx, int32_t my, int32_t lx, int32_t ly, HPEN pen) {
		gradient_fill.fill_rect(dc, { mx, my, lx + 1, ly + 1 }, settings.layer_border_color.right);
		::SelectObject(dc, pen);
	}
	static void __cdecl bottom(HDC dc, int32_t mx, int32_t my, int32_t lx, int32_t ly, HPEN pen) {
		gradient_fill.fill_rect(dc, { mx, my, lx + 1, ly + 1 }, settings.layer_border_color.bottom);
	}
	static void __cdecl separator(HDC dc, int32_t mx, int32_t my, int32_t lx, int32_t ly, HPEN pen) {
		gradient_fill.fill_rect(dc, { mx, my, lx + 1, ly + 1 }, settings.layer_border_color.separator);
		::SelectObject(dc, pen);
	}
};


////////////////////////////////
// 選択中オブジェクトのフォント．
////////////////////////////////
inline constinit struct {
	static HGDIOBJ WINAPI select_object_detour(HDC dc, HFONT font)
	{
		// note that settings.title_font_selected is not null here,
		// and draw_object(HDC, int) is hooked so the call to .is_selected() makes sense.
		return ::SelectObject(dc, draw_object.is_selected() ? settings.title_font_selected : font);
	}
} select_font;


////////////////////////////////
// AviUtlに渡す関数の定義．
////////////////////////////////
BOOL func_init(AviUtl::FilterPlugin* fp)
{
	// 情報源確保．
	if (!exedit.init(fp)) {
		::MessageBoxA(fp->hwnd, "拡張編集0.92が見つかりませんでした．",
			fp->name, MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	gradient_fill.dc_brush = static_cast<HBRUSH>(::GetStockObject(DC_BRUSH));

	// 設定ロード．
	load_settings(fp->dll_hinst);

	// 背景画像のロード．
	if (settings.overlay_non_selected || settings.overlay_selected) {
		image_loader loader{};
		auto src_dc = ::GetDC(exedit.fp->hwnd);

		overlay_non_selected.load(settings.overlay_non_selected, src_dc, loader);
		overlay_selected.load(settings.overlay_selected, src_dc, loader);

		::ReleaseDC(exedit.fp->hwnd, src_dc);
	}
	settings.overlay_non_selected.path = nullptr;
	settings.overlay_selected.path = nullptr;

	// 描画関数乗っ取り．
	if (settings.non_selected_border != settings.selected_border ||
		settings.title_font_selected != nullptr ||
		overlay_non_selected.is_effective() || overlay_selected.is_effective()) {
		for (auto ptr : exedit.call_draw_object)
			memory::hook_rel_call(ptr, &draw_object.draw_object_detour);
	}
	{
		auto const fn = gradient_fill.choose_grad(settings.gradient_back.type);
		for (auto ptr : exedit.call_fill_grad)
			memory::hook_rel_call(ptr, fn);
	}

	// フォント情報の上書き．
	if (settings.title_font != nullptr)
		memory::ProtectHelper::write(exedit.load_title_font, &settings.title_font);
	if (settings.title_font_selected != nullptr)
		memory::hook_api_call(exedit.call_select_obj_font, select_font.select_object_detour);

	// レイヤーの上下左右の枠線．
	if (settings.layer_border_color.left != CLR_INVALID)
		memory::hook_rel_call(exedit.DrawLayerBorderL, &LayerBorder::left);
	if (settings.layer_border_color.top != CLR_INVALID)
		memory::hook_rel_call(exedit.DrawLayerBorderT, &LayerBorder::top);
	if (settings.layer_border_color.right != CLR_INVALID)
		memory::hook_rel_call(exedit.DrawLayerBorderR, &LayerBorder::right);
	if (settings.layer_border_color.bottom != CLR_INVALID)
		memory::hook_rel_call(exedit.DrawLayerBorderB, &LayerBorder::bottom);
	if (settings.layer_border_color.separator != CLR_INVALID)
		memory::hook_rel_call(exedit.DrawLayerBorderSp, &LayerBorder::separator);

	// 時間範囲選択描画の色参照先を書き換え．
	if (settings.time_sel_color.tip != CLR_INVALID)
		memory::ProtectHelper::write(exedit.color_time_sel_tip, &settings.time_sel_color.tip);
	if (settings.time_sel_color.interior != CLR_INVALID)
		memory::ProtectHelper::write(exedit.color_time_sel_int, &settings.time_sel_color.interior);
	if (settings.time_sel_color.exterior != CLR_INVALID)
		memory::ProtectHelper::write(exedit.color_time_sel_ext, &settings.time_sel_color.exterior);

	// 点線枠の HPEN 参照先を書き換え．
	if (settings.hide_dot_border) {
		static constinit HPEN null_pen = nullptr;
		null_pen = static_cast<HPEN>(::GetStockObject(NULL_PEN));
		memory::ProtectHelper::write(exedit.SelectedObjectBorderPen, &null_pen);
	}

	return TRUE;
}

BOOL func_exit(AviUtl::FilterPlugin* fp)
{
	// delete fonts.
	auto delete_object = [](auto& handle) {
		if (handle != nullptr) {
			::DeleteObject(handle);
			handle = nullptr;
		}
	};
	delete_object(settings.title_font);
	delete_object(settings.title_font_selected);

	// delete cached images.
	overlay_non_selected.dispose();
	overlay_selected.dispose();

	return TRUE;
}


////////////////////////////////
// Entry point.
////////////////////////////////
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		::DisableThreadLibraryCalls(hinst);
		break;
	}
	return TRUE;
}


////////////////////////////////
// 看板．
////////////////////////////////
#define PLUGIN_NAME		"TLオブジェクト描画拡張"
#define PLUGIN_VERSION	"v1.01-beta1"
#define PLUGIN_AUTHOR	"sigma-axis"
#define PLUGIN_INFO_FMT(name, ver, author)	(name##" "##ver##" by "##author)
#define PLUGIN_INFO		PLUGIN_INFO_FMT(PLUGIN_NAME, PLUGIN_VERSION, PLUGIN_AUTHOR)

extern "C" __declspec(dllexport) AviUtl::FilterPluginDLL* __stdcall GetFilterTable(void)
{
	// （フィルタとは名ばかりの）看板．
	using Flag = AviUtl::FilterPlugin::Flag;
	static constinit AviUtl::FilterPluginDLL filter{
		.flag = Flag::NoConfig | Flag::AlwaysActive | Flag::ExInformation,
		.name = PLUGIN_NAME,

		.func_init = func_init,
		.func_exit = func_exit,
		.information = PLUGIN_INFO,
	};
	return &filter;
}

