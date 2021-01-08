//////////////////////////////////////////////////////////////////////
// Inline polygon render functions
//////////////////////////////////////////////////////////////////////

#if defined(POLY_INTENSITY) && !defined(POLY_UV)
s32 robj3d_findNextEdgeI(s32 xMinIndex, s32 xMin)
#elif !defined(POLY_INTENSITY) && defined(POLY_UV)
s32 robj3d_findNextEdgeT(s32 xMinIndex, s32 xMin)
#elif defined(POLY_INTENSITY) && defined(POLY_UV)
s32 robj3d_findNextEdgeTI(s32 xMinIndex, s32 xMin)
#else
s32 robj3d_findNextEdge(s32 xMinIndex, s32 xMin)
#endif
{
	s32 prevScanlineLen = s_edgeTopLength;
	s32 curIndex = xMinIndex;

	// The min and max indices should not match, otherwise it is an error.
	if (xMinIndex == s_polyMaxIndex)
	{
		s_edgeTopLength = prevScanlineLen;
		return -1;
	}

	while (1)
	{
		s32 nextIndex = curIndex + 1;
		if (nextIndex >= s_polyVertexCount) { nextIndex = 0; }
		else if (nextIndex < 0) { nextIndex = s_polyVertexCount - 1; }

		vec3_float* cur = &s_polyProjVtx[curIndex];
		vec3_float* next = &s_polyProjVtx[nextIndex];
		const s32 x0 = s32(cur->x + 0.5f);
		const s32 x1 = s32(next->x + 0.5f);
		const s32 y0 = s32(cur->y + 0.5f);
		const s32 y1 = s32(next->y + 0.5f);
		s32 dx = x1 - x0;
		if (x1 == s_maxScreenX)
		{
			dx++;
		}

		if (dx > 0)
		{
			s_edgeTopLength = dx;

			const f32 step = 1.0f / f32(dx);
			s_edgeTopY0_Pixel = y0;
			s_edgeTop_Y0 = f32(y0);

			const f32 dy = f32(y1 - y0);
			s_edgeTop_dYdX = dy * step;

			const f32 dz = next->z - cur->z;
			s_edgeTop_dZdX = dz * step;
			s_edgeTop_Z0 = cur->z;

			#if defined(POLY_INTENSITY)
				s_edgeTop_I0 = clamp(s_polyIntensity[curIndex], 0.0f, (f32)VSHADE_MAX_INTENSITY);
				const f32 dI = s_polyIntensity[nextIndex] - s_edgeTop_I0;
				s_edgeTop_dIdX = dI * step;
			#endif
			#if defined(POLY_UV)
				s_edgeTop_Uv0 = s_polyUv[curIndex];
				const f32 dU = s_polyUv[nextIndex].x - s_edgeTop_Uv0.x;
				const f32 dV = s_polyUv[nextIndex].z - s_edgeTop_Uv0.z;
				s_edgeTop_dUVdX.x = dU * step;
				s_edgeTop_dUVdX.z = dV * step;
			#endif

			s_edgeTopIndex = nextIndex;
			return 0;
		}
		else if (nextIndex == s_polyMaxIndex)
		{
			s_edgeTopLength = prevScanlineLen;
			return -1;
		}
		curIndex = nextIndex;
	}

	// This shouldn't be reached, but just in case.
	s_edgeTopLength = prevScanlineLen;
	return -1;
}

#if defined(POLY_INTENSITY) && !defined(POLY_UV)
s32 robj3d_findPrevEdgeI(s32 minXIndex)
#elif !defined(POLY_INTENSITY) && defined(POLY_UV)
s32 robj3d_findPrevEdgeT(s32 minXIndex)
#elif defined(POLY_INTENSITY) && defined(POLY_UV)
s32 robj3d_findPrevEdgeTI(s32 minXIndex)
#else
s32 robj3d_findPrevEdge(s32 minXIndex)
#endif
{
	const s32 len = s_edgeBotLength;
	s32 curIndex = minXIndex;
	if (minXIndex == s_polyMaxIndex)
	{
		s_edgeBotLength = len;
		return -1;
	}

	while (1)
	{
		s32 prevIndex = curIndex - 1;
		if (prevIndex >= s_polyVertexCount) { prevIndex = 0; }
		else if (prevIndex < 0) { prevIndex = s_polyVertexCount - 1; }

		vec3_float* cur = &s_polyProjVtx[curIndex];
		vec3_float* prev = &s_polyProjVtx[prevIndex];
		const s32 x0 = s32(cur->x + 0.5f);
		const s32 x1 = s32(prev->x + 0.5f);
		const s32 y0 = s32(cur->y + 0.5f);
		const s32 y1 = s32(prev->y + 0.5f);
		s32 dx = x1 - x0;
		if (s_maxScreenX == x1)
		{
			dx++;
		}

		if (dx > 0)
		{
			s_edgeBotLength = dx;

			const f32 step = 1.0f / f32(dx);
			s_edgeBotY0_Pixel = y0;
			s_edgeBot_Y0 = f32(y0);

			const s32 dy = y1 - y0;
			s_edgeBot_dYdX = f32(dy) * step;

			const f32 dz = prev->z - cur->z;
			s_edgeBot_dZdX = dz / f32(dx);
			s_edgeBot_Z0 = cur->z;
						
			#if defined(POLY_INTENSITY)
			s_edgeBot_I0 = clamp(s_polyIntensity[curIndex], 0.0f, (f32)VSHADE_MAX_INTENSITY);
			const f32 dI = s_polyIntensity[prevIndex] - s_edgeBot_I0;
			s_edgeBot_dIdX = dI * step;
			#endif
			#if defined(POLY_UV)
			s_edgeBot_Uv0 = s_polyUv[curIndex];
			const f32 dU = s_polyUv[prevIndex].x - s_edgeBot_Uv0.x;
			const f32 dV = s_polyUv[prevIndex].z - s_edgeBot_Uv0.z;
			s_edgeBot_dUVdX.x = dU*step;
			s_edgeBot_dUVdX.z = dV*step;
			#endif

			s_edgeBotIndex = prevIndex;

			return 0;
		}
		else
		{
			curIndex = prevIndex;
			if (prevIndex == s_polyMaxIndex)
			{
				s_edgeBotLength = len;
				return -1;
			}
		}
	}
	s_edgeBotLength = len;
	return -1;
}

#if !defined(POLY_INTENSITY) && !defined(POLY_UV)
void robj3d_drawColumnFlatColor()
{
	s32 end = s_columnHeight - 1;
	s32 offset = end * s_width;
	for (s32 i = end; i >= 0; i--, offset -= s_width)
	{
		s_pcolumnOut[offset] = s_polyColorIndex;
	}
}
#endif

#if defined(POLY_INTENSITY) && !defined(POLY_UV)
void robj3d_drawColumnShadedColor()
{
	const u8* colorMap = s_polyColorMap;

	fixed44_20 intensity = s_col_I0;
	u8  colorIndex = s_polyColorIndex;
	s32 dither = s_dither;

	s32 end = s_columnHeight - 1;
	s32 offset = end * s_width;
	for (s32 i = end; i >= 0; i--, offset -= s_width)
	{
		s32 pixelIntensity = floor20(intensity);
		if (dither)
		{
			const fixed44_20 iOffset = intensity - s_ditherOffset;
			if (iOffset >= 0)
			{
				pixelIntensity = floor20(iOffset);
			}
		}
		s_pcolumnOut[offset] = colorMap[(pixelIntensity&31)*256 + colorIndex];

		intensity += s_col_dIdY;
		dither = !dither;
	}
}
#endif

#if !defined(POLY_INTENSITY) && defined(POLY_UV)
void robj3d_drawColumnFlatTexture()
{
	const u8* colorMap = &s_polyColorMap[s_polyColorIndex * 256];
	const u8* textureData = s_polyTexture->frames[0].image;
	const s32 texHeight = s_polyTexture->frames[0].height;
	const s32 texWidthMask = s_polyTexture->frames[0].width - 1;
	const s32 texHeightMask = texHeight - 1;

	fixed44_20 U = s_col_Uv0.x;
	fixed44_20 V = s_col_Uv0.z;
#if defined(USE_FLOAT_Z_DIV)
	f32 Z = s_col_rZ0;
#else
	fixed44_20 Z = s_col_rZ0;
#endif
	
	s32 end = s_columnHeight - 1;
	s32 offset = end * s_width;

	const s32 N = s_affineCorrectionLen;
	if (s_perspectiveCorrect)
	{
		// Correct every 'N' pixels
		s32 len = s_columnHeight;
		s32 affineSpan = min(N, len);

	#if defined(USE_FLOAT_Z_DIV)
		const f32 z0 = 1.0f / Z;
		fixed44_20 S0 = fixed44_20(U * z0);
		fixed44_20 T0 = fixed44_20(V * z0);
	#else
		const fixed44_20 z0 = div20(ONE_20, Z);
		fixed44_20 S0 = mul20(U, z0);
		fixed44_20 T0 = mul20(V, z0);
	#endif

		Z += s_col_dZdY * affineSpan;
		U += s_col_dUVdY.x*affineSpan;
		V += s_col_dUVdY.z*affineSpan;

	#if defined(USE_FLOAT_Z_DIV)
		const f32 z1 = 1.0f / Z;
		fixed44_20 S1 = fixed44_20(U * z1);
		fixed44_20 T1 = fixed44_20(V * z1);
	#else
		const fixed44_20 z1 = div20(ONE_20, Z);
		fixed44_20 S1 = mul20(U, z1);
		fixed44_20 T1 = mul20(V, z1);
	#endif
		fixed44_20 S = S0, T = T0;
		fixed44_20 dSdY = (S1 - S0) / affineSpan;
		fixed44_20 dTdY = (T1 - T0) / affineSpan;

		for (s32 i = end; i >= 0; i--, offset -= s_width)
		{
			if (affineSpan <= 0 && len)
			{
				affineSpan = min(N, len);

				Z += s_col_dZdY * affineSpan;
				U += s_col_dUVdY.x*affineSpan;
				V += s_col_dUVdY.z*affineSpan;

				S0 = S1;
				T0 = T1;

			#if defined(USE_FLOAT_Z_DIV)
				const f32 z = 1.0f / Z;
				S1 = fixed44_20(U * z);
				T1 = fixed44_20(V * z);
			#else
				const fixed44_20 z = div20(ONE_20, Z);
				S1 = mul20(U, z);
				T1 = mul20(V, z);
			#endif

				S = S0; T = T0;
				dSdY = (S1 - S0) / affineSpan;
				dTdY = (T1 - T0) / affineSpan;
			}
			
			const u8 colorIndex = textureData[(floor20(S)&texWidthMask)*texHeight + (floor20(T)&texHeightMask)];
			s_pcolumnOut[offset] = colorMap[colorIndex];

			S += dSdY;
			T += dTdY;
			affineSpan--;
			len--;
		}
	}
	else
	{
		for (s32 i = end; i >= 0; i--, offset -= s_width)
		{
			const u8 colorIndex = textureData[(floor20(U)&texWidthMask)*texHeight + (floor20(V)&texHeightMask)];
			s_pcolumnOut[offset] = colorMap[colorIndex];

			U += s_col_dUVdY.x;
			V += s_col_dUVdY.z;
		}
	}
}
#endif

#if defined(POLY_INTENSITY) && defined(POLY_UV)
void robj3d_drawColumnShadedTexture()
{
	const u8* colorMap = s_polyColorMap;
	const u8* textureData = s_polyTexture->frames[0].image;
	const s32 texHeight = s_polyTexture->frames[0].height;
	const s32 texWidthMask = s_polyTexture->frames[0].width - 1;
	const s32 texHeightMask = texHeight - 1;

	fixed44_20 U = s_col_Uv0.x;
	fixed44_20 V = s_col_Uv0.z;
	fixed44_20 I = s_col_I0;
#if defined(USE_FLOAT_Z_DIV)
	f32 Z = s_col_rZ0;
#else
	fixed44_20 Z = s_col_rZ0;
#endif

	s32 end = s_columnHeight - 1;
	s32 offset = end * s_width;
	const s32 N = s_affineCorrectionLen;
	if (s_perspectiveCorrect)
	{
		// Correct every 'N' pixels
		s32 len = s_columnHeight;
		s32 affineSpan = min(N, len);

	#if defined(USE_FLOAT_Z_DIV)
		const f32 z0 = 1.0f / Z;
		fixed44_20 S0 = fixed44_20(U * z0);
		fixed44_20 T0 = fixed44_20(V * z0);
	#else
		const fixed44_20 z0 = div20(ONE_20, Z);
		fixed44_20 S0 = mul20(U, z0);
		fixed44_20 T0 = mul20(V, z0);
	#endif

		Z += s_col_dZdY * affineSpan;
		U += s_col_dUVdY.x*affineSpan;
		V += s_col_dUVdY.z*affineSpan;

	#if defined(USE_FLOAT_Z_DIV)
		const f32 z1 = 1.0f / Z;
		fixed44_20 S1 = fixed44_20(U * z1);
		fixed44_20 T1 = fixed44_20(V * z1);
	#else
		const fixed44_20 z1 = div20(ONE_20, Z);
		fixed44_20 S1 = mul20(U, z1);
		fixed44_20 T1 = mul20(V, z1);
	#endif
		fixed44_20 S = S0, T = T0;
		fixed44_20 dSdY = (S1 - S0) / affineSpan;
		fixed44_20 dTdY = (T1 - T0) / affineSpan;

		for (s32 i = end; i >= 0; i--, offset -= s_width)
		{
			if (affineSpan <= 0 && len)
			{
				affineSpan = min(N, len);

				Z += s_col_dZdY * affineSpan;
				U += s_col_dUVdY.x*affineSpan;
				V += s_col_dUVdY.z*affineSpan;

				S0 = S1;
				T0 = T1;

			#if defined(USE_FLOAT_Z_DIV)
				const f32 z = 1.0f / Z;
				S1 = fixed44_20(U * z);
				T1 = fixed44_20(V * z);
			#else
				const fixed44_20 z = div20(ONE_20, Z);
				S1 = mul20(U, z);
				T1 = mul20(V, z);
			#endif

				S = S0; T = T0;
				dSdY = (S1 - S0) / affineSpan;
				dTdY = (T1 - T0) / affineSpan;
			}

			const u8 colorIndex = textureData[(floor20(S)&texWidthMask)*texHeight + (floor20(T)&texHeightMask)];
			const s32 pixelIntensity = floor20(I)&31;
			s_pcolumnOut[offset] = colorMap[pixelIntensity*256 + colorIndex];

			I += s_col_dIdY;
			S += dSdY;
			T += dTdY;
			affineSpan--;
			len--;
		}
	}
	else
	{
		for (s32 i = end; i >= 0; i--, offset -= s_width)
		{
			const u8 colorIndex = textureData[(floor20(U)&texWidthMask)*texHeight + (floor20(V)&texHeightMask)];
			const s32 pixelIntensity = floor20(I)&31;
			s_pcolumnOut[offset] = colorMap[pixelIntensity*256 + colorIndex];

			I += s_col_dIdY;
			U += s_col_dUVdY.x;
			V += s_col_dUVdY.z;
		}
	}
}
#endif

#undef FIND_NEXT_EDGE
#undef FIND_PREV_EDGE
#undef DRAW_COLUMN
#if defined(POLY_INTENSITY) && !defined(POLY_UV)
#define FIND_NEXT_EDGE robj3d_findNextEdgeI
#define FIND_PREV_EDGE robj3d_findPrevEdgeI
#define DRAW_COLUMN robj3d_drawColumnShadedColor
#elif !defined(POLY_INTENSITY) && defined(POLY_UV)
#define FIND_NEXT_EDGE robj3d_findNextEdgeT
#define FIND_PREV_EDGE robj3d_findPrevEdgeT
#define DRAW_COLUMN robj3d_drawColumnFlatTexture
#elif defined(POLY_INTENSITY) && defined(POLY_UV)
#define FIND_NEXT_EDGE robj3d_findNextEdgeTI
#define FIND_PREV_EDGE robj3d_findPrevEdgeTI
#define DRAW_COLUMN robj3d_drawColumnShadedTexture
#else
#define FIND_NEXT_EDGE robj3d_findNextEdge
#define FIND_PREV_EDGE robj3d_findPrevEdge
#define DRAW_COLUMN robj3d_drawColumnFlatColor
#endif

#if defined(POLY_INTENSITY) && !defined(POLY_UV)
void robj3d_drawShadedColorPolygon(vec3_float* projVertices, f32* intensity, s32 vertexCount, u8 color)
#elif !defined(POLY_INTENSITY) && defined(POLY_UV)
void robj3d_drawFlatTexturePolygon(vec3_float* projVertices, vec2_float* uv, s32 vertexCount, Texture* texture, u8 color)
#elif defined(POLY_INTENSITY) && defined(POLY_UV)
void robj3d_drawShadedTexturePolygon(vec3_float* projVertices, vec2_float* uv, f32* intensity, s32 vertexCount, Texture* texture)
#else
void robj3d_drawFlatColorPolygon(vec3_float* projVertices, s32 vertexCount, u8 color)
#endif
{
	s32 xMax = INT_MIN;
	s32 xMin = INT_MAX;
	s_polyProjVtx = projVertices;
	s_polyVertexCount = vertexCount;

	#if defined(POLY_INTENSITY)
		s_polyIntensity = intensity;
	#endif
	#if defined(POLY_UV)
		s_polyUv = uv;
		s_polyTexture = texture;
	#endif

	s32 yMax = xMax;
	s32 yMin = xMin;
	if (vertexCount <= 0) { return; }

	// Compute the 2D bounding box of the polygon.
	// Track the extreme vertex indices in X.
	s32 minXIndex;
	vec3_float* projVertex = projVertices;
	for (s32 i = 0; i < s_polyVertexCount; i++, projVertex++)
	{
		const s32 x = s32(projVertex->x + 0.5f);
		if (x < xMin)
		{
			xMin = x;
			minXIndex = i;
		}
		if (x > xMax)
		{
			xMax = x;
			s_polyMaxIndex = i;
		}

		const s32 y = s32(projVertex->y + 0.5f);
		if (y < yMin) { yMin = y; }
		if (y > yMax) { yMax = y; }
	}

	// If the polygon is too small or off screen, skip it.
	if (xMin >= xMax || yMin > s_windowMaxY || yMax < s_windowMinY) { return; }

	s_polyColorMap = s_colorMap;
	#if !(defined(POLY_INTENSITY) && defined(POLY_UV))
		s_polyColorIndex = color;
	#endif
	#if defined(POLY_INTENSITY) && !defined(POLY_UV)
		s_ditherOffset = HALF_20;
	#endif
	s_columnX = xMin;

	if (FIND_NEXT_EDGE(minXIndex, xMin) != 0 || FIND_PREV_EDGE(minXIndex) != 0) { return; }

	for (s32 foundEdge = 0; !foundEdge && s_columnX >= s_minScreenX && s_columnX <= s_maxScreenX; s_columnX++)
	{
		const f32 edgeMinZ = s_perspectiveCorrect ? 1.0f / max(s_edgeBot_Z0, s_edgeTop_Z0) : min(s_edgeBot_Z0, s_edgeTop_Z0);
		const f32 z = s_depth1d[s_columnX];

		// Is ave edge Z occluded by walls? Is column outside of the vertical area?
		if (edgeMinZ < z && s_edgeTopY0_Pixel <= s_windowMaxY && s_edgeBotY0_Pixel >= s_windowMinY)
		{
			const s32 winTop = s_objWindowTop[s_columnX];
			const s32 winBot = s_objWindowBot[s_columnX];
			s32 y0_Top = s_edgeTopY0_Pixel;
			s32 y0_Bot = s_edgeBotY0_Pixel;
			#if defined(POLY_INTENSITY) || defined(POLY_UV)
				f32 yOffset = 0;
			#endif

			if (y0_Top < winTop)
			{
				y0_Top = winTop;
			}
			if (y0_Bot > winBot)
			{
				#if defined(POLY_INTENSITY) || defined(POLY_UV)
					yOffset = f32(y0_Bot - winBot);
				#endif
				y0_Bot = winBot;
			}

			s_columnHeight = y0_Bot - y0_Top + 1;
			if (s_columnHeight > 0)
			{
				const f32 height = f32(s_edgeBotY0_Pixel - s_edgeTopY0_Pixel + 1);
				s_pcolumnOut = &s_display[y0_Top*s_width + s_columnX];

				#if defined(POLY_INTENSITY)
					f32 col_dIdY = (s_edgeTop_I0 - s_edgeBot_I0) / height;
					f32 col_I0 = s_edgeBot_I0;
					if (yOffset)
					{
						col_I0 += (yOffset * s_col_dIdY);
					}
					s_col_dIdY = floatToFixed20(col_dIdY);
					s_col_I0 = floatToFixed20(col_I0);
					s_dither = ((s_columnX & 1) ^ (y0_Bot & 1)) - 1;
				#endif

				#if defined(POLY_UV)
					vec2_float dUVdY;
					dUVdY.x = (s_edgeTop_Uv0.x - s_edgeBot_Uv0.x) / height;
					dUVdY.z = (s_edgeTop_Uv0.z - s_edgeBot_Uv0.z) / height;
					vec2_float col_Uv0 = s_edgeBot_Uv0;
					if (yOffset != 0.0f)
					{
						col_Uv0.x += (yOffset * dUVdY.x);
						col_Uv0.z += (yOffset * dUVdY.z);
					}
					s_col_Uv0.x = floatToFixed20(col_Uv0.x);
					s_col_Uv0.z = floatToFixed20(col_Uv0.z);
					s_col_dUVdY.x = floatToFixed20(dUVdY.x);
					s_col_dUVdY.z = floatToFixed20(dUVdY.z);
				
					if (s_perspectiveCorrect)
					{
						f32 col_rZ0 = s_edgeBot_Z0;
						f32 col_dZdY = (s_edgeTop_Z0 - s_edgeBot_Z0) / height;
						if (yOffset != 0.0f)
						{
							col_rZ0 += (yOffset * col_dZdY);
						}
					#if defined(USE_FLOAT_Z_DIV)
						s_col_dZdY = col_dZdY;
						s_col_rZ0 = col_rZ0;
					#else
						s_col_dZdY = floatToFixed20(col_dZdY);
						s_col_rZ0 = floatToFixed20(col_rZ0);
					#endif
					}
				#endif

				DRAW_COLUMN();
			}
		}

		s_edgeTopLength--;
		if (s_edgeTopLength <= 0)
		{
			foundEdge = FIND_NEXT_EDGE(s_edgeTopIndex, s_columnX);
		}
		else
		{
			#if defined(POLY_INTENSITY)
				s_edgeTop_I0 = clamp(s_edgeTop_I0 + s_edgeTop_dIdX, 0.0f, (f32)VSHADE_MAX_INTENSITY);
			#endif
			#if defined(POLY_UV)
				s_edgeTop_Uv0.x += s_edgeTop_dUVdX.x;
				s_edgeTop_Uv0.z += s_edgeTop_dUVdX.z;
			#endif

			s_edgeTop_Y0 += s_edgeTop_dYdX;
			s_edgeTop_Z0 += s_edgeTop_dZdX;
			s_edgeTopY0_Pixel = roundFloat(s_edgeTop_Y0);
		}
		if (foundEdge == 0)
		{
			s_edgeBotLength--;
			if (s_edgeBotLength <= 0)
			{
				foundEdge = FIND_PREV_EDGE(s_edgeBotIndex);
			}
			else
			{
				#if defined(POLY_INTENSITY)
				s_edgeBot_I0 = clamp(s_edgeBot_I0 + s_edgeBot_dIdX, 0.0f, (f32)VSHADE_MAX_INTENSITY);
				#endif
				#if defined(POLY_UV)
					s_edgeBot_Uv0.x += s_edgeBot_dUVdX.x;
					s_edgeBot_Uv0.z += s_edgeBot_dUVdX.z;
				#endif

				s_edgeBot_Y0 += s_edgeBot_dYdX;
				s_edgeBot_Z0 += s_edgeBot_dZdX;
				s_edgeBotY0_Pixel = roundFloat(s_edgeBot_Y0);
			}
		}
	}
}
