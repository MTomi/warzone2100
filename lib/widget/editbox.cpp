/*
	This file is part of Warzone 2100.
	Copyright (C) 1999-2004  Eidos Interactive
	Copyright (C) 2005-2011  Warzone 2100 Project

	Warzone 2100 is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	Warzone 2100 is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Warzone 2100; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/
/** @file
 *  Functions for the edit box widget.
 */

#include <string.h>

#include "lib/framework/frame.h"
#include "lib/framework/utf.h"
#include "lib/framework/wzapp_c.h"
#include "widget.h"
#include "widgint.h"
#include "editbox.h"
#include "form.h"
// FIXME Direct iVis implementation include!
#include "lib/ivis_opengl/pieblitfunc.h"
#include "lib/ivis_opengl/textdraw.h"
#include "scrap.h"


/* Pixel gap between edge of edit box and text */
#define WEDB_XGAP	4
#define WEDB_YGAP	2

/* Size of the overwrite cursor */
#define WEDB_CURSORSIZE		8

/* Whether the cursor blinks or not */
#define CURSOR_BLINK		1

/* The time the cursor blinks for */
#define WEDB_BLINKRATE		800

/* Number of characters to jump the edit box text when moving the cursor */
#define WEDB_CHARJUMP		6


W_EDBINIT::W_EDBINIT()
	: pText(NULL)
	, FontID(font_regular)
	, pBoxDisplay(NULL)
	, pFontDisplay(NULL)
{}

W_EDITBOX::W_EDITBOX(W_EDBINIT const *init)
	: WIDGET(init, WIDG_EDITBOX)
	, FontID(init->FontID)
	, blinkOffset(wzGetTicks())
	, pBoxDisplay(init->pBoxDisplay)
	, pFontDisplay(init->pFontDisplay)
	, HilightAudioID(WidgGetHilightAudioID())
	, ClickedAudioID(WidgGetClickedAudioID())
	, AudioCallback(WidgGetAudioCallback())
{
	char const *text = init->pText;
	if (!text)
	{
		text = "";
	}
	aText = QString::fromUtf8(text);

	if (display == NULL)
	{
		display = editBoxDisplay;
	}

	initialise();
}

/* Create an edit box widget data structure */
W_EDITBOX* editBoxCreate(const W_EDBINIT* psInit)
{
	if (psInit->style & ~(WEDB_PLAIN | WIDG_HIDDEN))
	{
		ASSERT( false, "Unknown edit box style" );
		return NULL;
	}

	/* Allocate the required memory */
	W_EDITBOX *psWidget = new W_EDITBOX(psInit);
	if (psWidget == NULL)
	{
		debug(LOG_FATAL, "editBoxCreate: Out of memory");
		abort();
		return NULL;
	}

	return psWidget;
}


/* Initialise an edit box widget */
void W_EDITBOX::initialise()
{
	state = WEDBS_FIXED;
	printStart = 0;
	iV_SetFont(FontID);
	fitStringStart();
}


/* Insert a character into a text buffer */
void W_EDITBOX::insertChar(QChar ch)
{
	if (ch == '\0') return;

	ASSERT(insPos <= aText.length(), "Invalid insertion point");

	/* Move the end of the string up by one (including terminating \0) */
	/* Insert the character */
	aText.insert(insPos, ch);

	/* Update the insertion point */
	++insPos;
}


/* Put a character into a text buffer overwriting any text under the cursor */
void W_EDITBOX::overwriteChar(QChar ch)
{
	if (ch == '\0') return;

	ASSERT(insPos <= aText.length(), "overwriteChar: Invalid insertion point");

	if (insPos == aText.length())
	{
		// At end of string.
		insertChar(ch);
		return;
	}

	/* Store the character */
	aText[insPos] = ch;

	/* Update the insertion point */
	++insPos;
}

/* Put a character into a text buffer overwriting any text under the cursor */
void W_EDITBOX::putSelection()
{
	static char* scrap = NULL;
	int scraplen;

	get_scrap(T('T','E','X','T'), &scraplen, &scrap);
	if (scraplen > 0 && scraplen < WIDG_MAXSTR-2)
	{
		aText = QString::fromUtf8(scrap);
		insPos = aText.length();
	}
}


/* Delete a character to the right of the position */
void W_EDITBOX::delCharRight()
{
	ASSERT(insPos <= aText.length(), "Invalid deletion point");

	/* Can't delete if we are at the end of the string */
	/* Move the end of the string down by one */
	aText.remove(insPos, 1);
}


/* Delete a character to the left of the position */
void W_EDITBOX::delCharLeft()
{
	/* Can't delete if we are at the start of the string */
	if (insPos == 0)
	{
		return;
	}

	--insPos;
	delCharRight();
}


/* Calculate how much of the start of a string can fit into the edit box */
void W_EDITBOX::fitStringStart()
{
	// We need to calculate the whole string's pixel size.
	// From QuesoGLC's notes: additional processing like kerning creates strings of text whose dimensions are not directly
	// related to the simple juxtaposition of individual glyph metrics. For example, the advance width of "VA" isn't the
	// sum of the advances of "V" and "A" taken separately.
	QString tmp = aText;
	tmp.remove(0, printStart);  // Ignore the first printStart characters.

	while (!tmp.isEmpty())
	{
		int pixelWidth = iV_GetTextWidth(tmp.toUtf8().constData());

		if (pixelWidth <= width - (WEDB_XGAP*2 + WEDB_CURSORSIZE))
		{
			printChars = tmp.length();
			printWidth = pixelWidth;
			return;
		}

		tmp.remove(tmp.length() - 1, 1);  // Erase last char.
	}

	printChars = 0;
	printWidth = 0;
}


/* Calculate how much of the end of a string can fit into the edit box */
void W_EDITBOX::fitStringEnd()
{
	QString tmp = aText;

	printStart = 0;

	while (!tmp.isEmpty())
	{
		int pixelWidth = iV_GetTextWidth(tmp.toUtf8().constData());

		if (pixelWidth <= width - (WEDB_XGAP*2 + WEDB_CURSORSIZE))
		{
			printChars = tmp.length();
			printWidth = pixelWidth;
			return;
		}

		tmp.remove(0, 1);  // Erase first char.
		++printStart;
	}

	printChars = 0;
	printWidth = 0;
}


/* Calculate how much of the end of a string can fit into the edit box */
void W_EDITBOX::setCursorPosPixels(int xPos)
{
	QString tmp = aText;
	tmp.remove(0, printStart);  // Consider only the visible text.
	tmp.remove(printChars, tmp.length());

	int prevDelta = INT32_MAX;
	int prevPos = printStart + tmp.length();
	while (!tmp.isEmpty())
	{
		int pixelWidth = iV_GetTextWidth(tmp.toUtf8().constData());
		int delta = pixelWidth - (xPos - (WEDB_XGAP + WEDB_CURSORSIZE/2));
		int pos = printStart + tmp.length();

		if (delta <= 0)
		{
			insPos = -delta < prevDelta? pos : prevPos;
			return;
		}

		tmp.remove(tmp.length() - 1, 1);  // Erase last char.

		prevDelta = delta;
		prevPos = pos;
	}

	insPos = printStart;
}


void W_EDITBOX::run(W_CONTEXT *psContext)
{
	/* Note the edit state */
	unsigned editState = state & WEDBS_MASK;

	/* Only have anything to do if the widget is being edited */
	if ((editState & WEDBS_MASK) == WEDBS_FIXED)
	{
		return;
	}

	/* If there is a mouse click outside of the edit box - stop editing */
	int mx = psContext->mx;
	int my = psContext->my;
	if (mousePressed(MOUSE_LMB) && (mx < x || mx > x + width || my < y || my > y + height))
	{
		screenClearFocus(psContext->psScreen);
		return;
	}

	/* note the widget state */
	iV_SetFont(FontID);

	/* Loop through the characters in the input buffer */
	bool done = false;
	utf_32_char unicode;
	for (unsigned key = inputGetKey(&unicode); key != 0 && !done; key = inputGetKey(&unicode))
	{
		// Don't blink while typing.
		blinkOffset = wzGetTicks();

		int len = 0;

		/* Deal with all the control keys, assume anything else is a printable character */
		switch (key)
		{
		case INPBUF_LEFT :
			/* Move the cursor left */
			insPos = MAX(insPos - 1, 0);

			/* If the cursor has gone off the left of the edit box,
			 * need to update the printable text.
			 */
			if (insPos < printStart)
			{
				printStart = MAX(printStart - WEDB_CHARJUMP, 0);
				fitStringStart();
			}
			debug(LOG_INPUT, "EditBox cursor left");
			break;
		case INPBUF_RIGHT :
			/* Move the cursor right */
			len = aText.length();
			insPos = MIN(insPos + 1, len);

			/* If the cursor has gone off the right of the edit box,
			 * need to update the printable text.
			 */
			if (insPos > printStart + printChars)
			{
				printStart = MIN(printStart + WEDB_CHARJUMP, len - 1);
				fitStringStart();
			}
			debug(LOG_INPUT, "EditBox cursor right");
			break;
		case INPBUF_UP :
			debug(LOG_INPUT, "EditBox cursor up");
			break;
		case INPBUF_DOWN :
			debug(LOG_INPUT, "EditBox cursor down");
			break;
		case INPBUF_HOME :
			/* Move the cursor to the start of the buffer */
			insPos = 0;
			printStart = 0;
			fitStringStart();
			debug(LOG_INPUT, "EditBox cursor home");
			break;
		case INPBUF_END :
			/* Move the cursor to the end of the buffer */
			insPos = aText.length();
			if (insPos != printStart + printChars)
			{
				fitStringEnd();
			}
			debug(LOG_INPUT, "EditBox cursor end");
			break;
		case INPBUF_INS :
			if (editState == WEDBS_INSERT)
			{
				editState = WEDBS_OVER;
			}
			else
			{
				editState = WEDBS_INSERT;
			}
			debug(LOG_INPUT, "EditBox cursor insert");
			break;
		case INPBUF_DEL :
			delCharRight();

			/* Update the printable text */
			fitStringStart();
			debug(LOG_INPUT, "EditBox cursor delete");
			break;
		case INPBUF_PGUP :
			debug(LOG_INPUT, "EditBox cursor page up");
			break;
		case INPBUF_PGDN :
			debug(LOG_INPUT, "EditBox cursor page down");
			break;
		case INPBUF_BKSPACE :
			/* Delete the character to the left of the cursor */
			delCharLeft();

			/* Update the printable text */
			if (insPos <= printStart)
			{
				printStart = MAX(printStart - WEDB_CHARJUMP, 0);
			}
			fitStringStart();
			debug(LOG_INPUT, "EditBox cursor backspace");
			break;
		case INPBUF_TAB :
			putSelection();

			/* Update the printable text */
			fitStringEnd();
			debug(LOG_INPUT, "EditBox cursor tab");
			break;
		case INPBUF_CR :
		case SDLK_KP_ENTER:		// either normal return key || keypad enter
			/* Finish editing */
			focusLost(psContext->psScreen);
			screenClearFocus(psContext->psScreen);
			debug(LOG_INPUT, "EditBox cursor return");
			return;
			break;
		case INPBUF_ESC :
			debug(LOG_INPUT, "EditBox cursor escape");
			break;

		default:
			/* Dealt with everything else this must be a printable character */
			if (editState == WEDBS_INSERT)
			{
				insertChar(unicode);
			}
			else
			{
				overwriteChar(unicode);
			}
			len = aText.length();
			/* Update the printable chars */
			if (insPos == len)
			{
				fitStringEnd();
			}
			else
			{
				fitStringStart();
				if (insPos > printStart + printChars)
				{
					printStart = MIN(printStart + WEDB_CHARJUMP, len - 1);
					if (printStart >= len)
					{
						fitStringStart();
					}
				}
			}
			break;
		}
	}

	/* Store the current widget state */
	state = (state & ~WEDBS_MASK) | editState;
}


/* Set the current string for the edit box */
void W_EDITBOX::setString(char const *utf8)
{
	aText = QString::fromUtf8(utf8);
	initialise();
}


/* Respond to a mouse click */
void W_EDITBOX::clicked(W_CONTEXT *psContext, WIDGET_KEY)
{
	if (state & WEDBS_DISABLE)  // disabled button.
	{
		return;
	}

	// Set cursor position to the click location.
	iV_SetFont(FontID);
	setCursorPosPixels(psContext->mx - x);

	// Cursor should be visible instantly.
	blinkOffset = wzGetTicks();

	if ((state & WEDBS_MASK) == WEDBS_FIXED)
	{
		if (AudioCallback)
		{
			AudioCallback(ClickedAudioID);
		}

		/* Set up the widget state */
		state = (state & ~WEDBS_MASK) | WEDBS_INSERT;

		/* Calculate how much of the string can appear in the box */
		fitStringEnd();

		/* Clear the input buffer */
		inputClearBuffer();

		/* Tell the form that the edit box has focus */
		screenSetFocus(psContext->psScreen, this);
	}
}


/* Respond to loss of focus */
void W_EDITBOX::focusLost(W_SCREEN *psScreen)
{
	ASSERT(!(state & WEDBS_DISABLE), "editBoxFocusLost: disabled edit box" );

	/* Stop editing the widget */
	state = WEDBS_FIXED;
	printStart = 0;
	fitStringStart();

	widgSetReturn(psScreen, this);
}


/* Respond to a mouse moving over an edit box */
void editBoxHiLite(W_EDITBOX *psWidget)
{
	if(psWidget->state & WEDBS_DISABLE)
	{
		return;
	}

	if(psWidget->AudioCallback)
	{
		psWidget->AudioCallback(psWidget->HilightAudioID);
	}

	psWidget->state |= WEDBS_HILITE;
}


/* Respond to the mouse moving off an edit box */
void editBoxHiLiteLost(W_EDITBOX *psWidget)
{
	if(psWidget->state & WEDBS_DISABLE)
	{
		return;
	}

	psWidget->state = psWidget->state & WEDBS_MASK;
}


/* The edit box display function */
void editBoxDisplay(WIDGET *psWidget, UDWORD xOffset, UDWORD yOffset, PIELIGHT *pColours)
{
	W_EDITBOX	*psEdBox;
	SDWORD		x0,y0,x1,y1, fx,fy, cx,cy;
	enum iV_fonts CurrFontID;

#if CURSOR_BLINK
	bool		blink;
#endif

	psEdBox = (W_EDITBOX *)psWidget;
	CurrFontID = psEdBox->FontID;

	x0=psEdBox->x + xOffset;
	y0=psEdBox->y + yOffset;
	x1=x0 + psEdBox->width;
	y1=y0 + psEdBox->height;

	if(psEdBox->pBoxDisplay)
	{
		psEdBox->pBoxDisplay((WIDGET *)psEdBox, xOffset, yOffset, pColours);
	}
	else
	{
		pie_BoxFill(x0, y0, x1, y1, pColours[WCOL_BKGRND]);

		iV_Line(x0,y0, x1,y0, pColours[WCOL_DARK]);
		iV_Line(x0,y0, x0,y1, pColours[WCOL_DARK]);
		iV_Line(x0,y1, x1,y1, pColours[WCOL_LIGHT]);
		iV_Line(x1,y1, x1,y0, pColours[WCOL_LIGHT]);
	}

	fx = x0 + WEDB_XGAP;// + (psEdBox->width - fw) / 2;

	iV_SetFont(CurrFontID);
	iV_SetTextColour(pColours[WCOL_TEXT]);

	fy = y0 + (psEdBox->height - iV_GetTextLineSize())/2 - iV_GetTextAboveBase();

	/* If there is more text than will fit into the box, display the bit with the cursor in it */
	QString tmp = psEdBox->aText;
	tmp.remove(0, psEdBox->printStart);  // Erase anything there isn't room to display.
	tmp.remove(psEdBox->printChars, tmp.length());

//	if(psEdBox->pFontDisplay) {
//		psEdBox->pFontDisplay(fx,fy, pPrint);
//	} else {
		iV_DrawText(tmp.toUtf8().constData(), fx, fy);
//	}

	// Display the cursor if editing
#if CURSOR_BLINK
	blink = !(((SDL_GetTicks() - psEdBox->blinkOffset)/WEDB_BLINKRATE) % 2);
	if ((psEdBox->state & WEDBS_MASK) == WEDBS_INSERT && blink)
#else
	if ((psEdBox->state & WEDBS_MASK) == WEDBS_INSERT)
#endif
	{
		// insert mode
		QString tmp = psEdBox->aText;
		tmp.remove(psEdBox->insPos, tmp.length());         // Erase from the cursor on, to find where the cursor should be.
		tmp.remove(0, psEdBox->printStart);

		cx = x0 + WEDB_XGAP + iV_GetTextWidth(tmp.toUtf8().constData());
		cx += iV_GetTextWidth("-");
		cy = fy;
		iV_Line(cx, cy + iV_GetTextAboveBase(), cx, cy - iV_GetTextBelowBase(), pColours[WCOL_CURSOR]);
	}
#if CURSOR_BLINK
	else if ((psEdBox->state & WEDBS_MASK) == WEDBS_OVER && blink)
#else
	else if ((psEdBox->state & WEDBS_MASK) == WEDBS_OVER)
#endif
	{
		// overwrite mode
		QString tmp = psEdBox->aText;
		tmp.remove(psEdBox->insPos, tmp.length());         // Erase from the cursor on, to find where the cursor should be.
		tmp.remove(0, psEdBox->printStart);

		cx = x0 + WEDB_XGAP + iV_GetTextWidth(tmp.toUtf8().constData());
		cy = fy;
		iV_Line(cx, cy, cx + WEDB_CURSORSIZE, cy, pColours[WCOL_CURSOR]);
	}

	if(psEdBox->pBoxDisplay == NULL)
	{
		if (psEdBox->state & WEDBS_HILITE)
		{
			/* Display the button hilite */
			iV_Line(x0-2,y0-2, x1+2,y0-2, pColours[WCOL_HILITE]);
			iV_Line(x0-2,y0-2, x0-2,y1+2, pColours[WCOL_HILITE]);
			iV_Line(x0-2,y1+2, x1+2,y1+2, pColours[WCOL_HILITE]);
			iV_Line(x1+2,y1+2, x1+2,y0-2, pColours[WCOL_HILITE]);
		}
	}
}



/* Set an edit box'sstate */
void editBoxSetState(W_EDITBOX *psEditBox, UDWORD state)
{
	if (state & WEDBS_DISABLE)
	{
		psEditBox->state |= WEDBS_DISABLE;
	}
	else
	{
		psEditBox->state &= ~WEDBS_DISABLE;
	}

}
