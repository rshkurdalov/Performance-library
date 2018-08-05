// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma comment(lib, "dependencies/freetype/freetype.lib")

#include "graphics\Font.h"
#include "GeometryPath.h"
#include "freetype\ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <fstream>
#include <vector>

namespace graphics
{
	FT_Library ftLibrary;

	HResult FtInitialize()
	{
		CheckReturnFail(FT_Init_FreeType(&ftLibrary));
		return HResultSuccess;
	}

	HResult LoadFont(
		std::wstring path,
		Font **ppFont)
	{
		std::fstream file(path.c_str(), std::ios::in | std::ios::binary);
		if (!file.is_open()) return HResultInvalidArg;
		std::vector<int8> data;
		file.seekg(0, std::ios::end);
		data.resize(file.tellg());
		file.seekg(0, std::ios::beg);
		file.read(data.data(), data.size());
		file.close();

		FT_Face ftFace;
		CheckReturnFail(FT_New_Memory_Face(
			ftLibrary,
			(FT_Byte *)data.data(),
			(FT_Long)data.size(),
			0,
			&ftFace));
		FT_Set_Char_Size(ftFace, ftFace->units_per_EM, ftFace->units_per_EM, 0, 0);
		float32 sm = 1.0f / ftFace->units_per_EM;
		std::map<wchar, CharMetadata> charData;
		GeometryPath *g;
		Vector2f v2, v3, v4, vt;
		uint32 idx, code, p, pf, pl, pNext, pNext2;
		code = FT_Get_First_Char(ftFace, &idx);
		while (idx != 0)
		{
			if (code == L'n')
			{
				idx = idx;
			}
			FT_Load_Glyph(ftFace, idx, FT_LOAD_NO_BITMAP | FT_LOAD_IGNORE_TRANSFORM);
			g = new GeometryPath();
			p = 0;
			pf = 0;
			for (uint32 ct = 0; ct < ftFace->glyph->outline.n_contours; ct++)
			{
				pl = ftFace->glyph->outline.contours[ct];
				while (p <= pl)
				{
					vt = Vector2f(
						ftFace->glyph->outline.points[p].x*sm,
						-ftFace->glyph->outline.points[p].y*sm);
					if (ftFace->glyph->outline.tags[p] & FT_CURVE_TAG_ON)
					{
						if (p == pf) g->Move(vt);
						else g->AddLine(vt);
						p++;
					}
					else
					{
						pNext = (p == pl ? pf : p + 1);
						v2 = Vector2f(
							ftFace->glyph->outline.points[pNext].x*sm,
							-ftFace->glyph->outline.points[pNext].y*sm);
						pNext2 = (pNext == pl ? pf : pNext + 1);
						v3 = Vector2f(
							ftFace->glyph->outline.points[pNext2].x*sm,
							-ftFace->glyph->outline.points[pNext2].y*sm);
						if (ftFace->glyph->outline.tags[p] & FT_CURVE_TAG_CUBIC)
						{
							g->AddCubicBezier(vt, v2, v3);
							p += 3;
						}
						else
						{
							if ((ftFace->glyph->outline.tags[pNext] & 3) == FT_CURVE_TAG_CONIC)
							{
								v4 = (vt + v2)*0.5f;
								g->AddQuadraticBezier(vt, v4);
								g->AddQuadraticBezier(v2, v3);
								p += 3;
							} 
							else
							{
								g->AddQuadraticBezier(vt, v2);
								p += 2;
							}
						}
					}
				}
				p = pl + 1;
				pf = pl + 1;
			}
			CharMetadata charMetadata;
			charMetadata.code = code;
			charMetadata.advance.x = ftFace->glyph->advance.x*sm;
			charMetadata.advance.y = ftFace->glyph->advance.y*sm;
			charMetadata.outline = g;
			charData[code] = charMetadata;
			code = FT_Get_Next_Char(ftFace, code, &idx);
			if (charData.size() >= 200) break;
		}

		*ppFont = new Font(charData);

		return HResultSuccess;
	}
	Font::Font(std::map<wchar, CharMetadata> &charData)
	{
		this->charData = charData;
	}
	Font::~Font()
	{

	}
}
