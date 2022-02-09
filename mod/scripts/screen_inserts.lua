local inserts =
{
	{
		"main-menu.lua",
		{ "widgets" },
		{
			name = [[modManBtn]],
			isVisible = true,
			noInput = false,
			anchor = 1,
			rotation = 0,
			x = 420,
			xpx = true,
			y = 209,
			ypx = true,
			w = 250,
			wpx = true,
			h = 40,
			hpx = true,
			sx = 1,
			sy = 1,
			ctor = [[button]],
			clickSound = [[SpySociety/HUD/menu/click]],
			hoverSound = [[SpySociety/HUD/menu/rollover]],
			hoverScale = 1,
			halign = MOAITextBox.LEFT_JUSTIFY,
			valign = MOAITextBox.CENTER_JUSTIFY,
			text_style = [[font1_16_r]],
			offset =
			{
				x = 10,
				xpx = true,
				y = 0,
				ypx = true,
			},
			images =
			{
				{
					file = [[white.png]],
					name = [[inactive]],
					color =
					{
						0.243137255311012,
						0.423529416322708,
						0.423529416322708,
						0.705882370471954,
					},
				},
				{
					file = [[white.png]],
					name = [[hover]],
					color =
					{
						0.549019634723663,
						1,
						1,
						0.705882370471954,
					},
				},
				{
					file = [[white.png]],
					name = [[active]],
					color =
					{
						0.549019634723663,
						1,
						1,
						0.705882370471954,
					},
				},
			},
		},
	}
}

return inserts
