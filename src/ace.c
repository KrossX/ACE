#include <windows.h>

#define IDOK     1
#define IDCANCEL 2
#define IDAPPLY  3

#define IDCB_PRESET    1000
#define IDCB_AAMODE    1001
#define IDCB_AASAMPLE  1002
#define IDCB_AAFILTER  1003
#define IDCB_AAMETHOD  1004
#define IDCB_MLAA      1005
#define IDCB_AFMODE    1006
#define IDCB_AFLEVEL   1007
#define IDCB_TFQ       1008
#define IDCB_SFO       1009
#define IDCB_VSYNC     1010
#define IDCB_TBUFFER   1011
#define IDCB_TESSMODE  1012
#define IDCB_TESSLEVEL 1013

HINSTANCE instance;

char regkey[] = "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\0000\\UMD";

struct ati_settings
{
	int main3d;
	int anti_alias, anti_alias_samples, aaf, mlf, ase, asd, astt;
	int aniso_type, aniso_degree;
	int texture_lod;
	int tfq;
	int vsync_control;
	int tessellation, tessellation_option;
	int surface_format_replacements;
	int triple_buffering;
} ati;

int aa_to_sel(int ati_value)
{
	switch(ati_value)
	{
		case 0: return 0;
		case 2: return 1;
		case 4: return 2;
		case 8: return 3;
	}

	return 0;
}

int sel_to_aa(int sel_value)
{
	switch(sel_value)
	{
		case 0: return 0;
		case 1: return 2;
		case 2: return 4;
		case 3: return 8;
	}

	return 0;
}

int aniso_to_sel(int ati_value)
{
	switch(ati_value)
	{
		case 2:  return 0;
		case 4:  return 1;
		case 8:  return 2;
		case 16: return 3;
	}

	return 0;
}

int sel_to_aniso(int sel_value)
{
	switch(sel_value)
	{
		case 0: return  2;
		case 1: return  4;
		case 2: return  8;
		case 3: return 16;
	}

	return 0;
}

int tess_to_sel(int ati_value)
{
	switch(ati_value)
	{
		case 0:  return 0;
		case 2:  return 1;
		case 4:  return 2;
		case 6:  return 3;
		case 8:  return 4;
		case 16: return 5;
		case 32: return 6;
		case 64: return 7;
	}

	return 0;
}

int sel_to_tess(int sel_value)
{
	switch(sel_value)
	{
		case 0: return 0;
		case 1: return 2;
		case 2: return 4;
		case 3: return 6;
		case 4: return 8;
		case 5: return 16;
		case 6: return 32;
		case 7: return 64;
	}

	return 0;
}

int reg_get_value(HKEY key, char *name)
{
	char buf[16] = {0};
	DWORD len = 16;

	if(RegQueryValueExA(key, name, 0, 0, (void*)buf, &len) == ERROR_SUCCESS)
	{
		char *str = buf;
		int val  = 0;
		int sign = 0;

		while(len--)
		{
			char c = *str++;

			if(c == 0x2D)
			{
				sign = 1;
			}
			else if(c >= 0x30 && c <= 0x39)
			{
				val = val * 10 + (c - 0x30);
			}
		}

		return sign ? -val : val;
	}
	else
	{
		return 0;
	}
}

void reg_set_value(HKEY key, char *name, int val)
{
	char buf[16];
	int len = wsprintfA(buf, "%d", val);
	int tmp = len;

	while(tmp--)
	{
		buf[tmp * 2]     = buf[tmp];
		buf[tmp * 2 + 1] = 0;
	}

	RegSetValueExA(key, name, 0, REG_BINARY, buf, len * 2);
}

LONG load_settings(void)
{
	HKEY key;
	LONG res = RegOpenKeyExA(HKEY_LOCAL_MACHINE, regkey, 0, KEY_READ, &key);

	if(res == ERROR_SUCCESS)
	{
		ati.main3d             = reg_get_value(key, "Main3D");
		ati.anti_alias         = reg_get_value(key, "AntiAlias");
		ati.anti_alias_samples = reg_get_value(key, "AntiAliasSamples");
		ati.mlf                = reg_get_value(key, "MLF");
		ati.aaf                = reg_get_value(key, "AAF");
		ati.ase                = reg_get_value(key, "ASE");
		ati.asd                = reg_get_value(key, "ASD");
		ati.astt               = reg_get_value(key, "ASTT");

		ati.aniso_degree = reg_get_value(key, "AnisoDegree");
		//ati.texture_lod  = reg_get_value(key, "TextureLod");
		ati.tfq          = reg_get_value(key, "TFQ");

		ati.vsync_control       = reg_get_value(key, "VSyncControl");
		ati.tessellation        = reg_get_value(key, "Tessellation");
		ati.tessellation_option = reg_get_value(key, "Tessellation_OPTION");

		ati.surface_format_replacements = reg_get_value(key, "SurfaceFormatReplacements");
		ati.triple_buffering            = reg_get_value(key, "EnableTripleBuffering");
	}
	else
	{
		MessageBoxA(NULL, "Coud not load settings from registry.", "Error!", MB_OK);
	}

	return res;
}

LONG save_settings(HWND wnd)
{
	HKEY key;
	LONG res = RegOpenKeyExA(HKEY_LOCAL_MACHINE, regkey, 0, KEY_WRITE, &key);

	if(res == ERROR_SUCCESS)
	{
		//                     = SendMessageA(GetDlgItem(wnd,IDCB_PRESET),    CB_GETCURSEL, 0, 0);
		ati.anti_alias         = SendMessageA(GetDlgItem(wnd,IDCB_AAMODE),    CB_GETCURSEL, 0, 0);
		ati.anti_alias_samples = SendMessageA(GetDlgItem(wnd,IDCB_AASAMPLE),  CB_GETCURSEL, 0, 0);

		if(ati.anti_alias == 0)
		{
			ati.anti_alias         = 1;
			ati.anti_alias_samples = 0;
		}
		else
		{
			ati.anti_alias         = 2;
			ati.anti_alias_samples = sel_to_aa(ati.anti_alias_samples);
		}

		ati.asd          = SendMessageA(GetDlgItem(wnd,IDCB_AAMETHOD),  CB_GETCURSEL, 0, 0);

		switch(ati.asd)
		{
			case 0:
				ati.asd  = -1;
				ati.ase  =  0;
				ati.astt =  0;
				break;

			case 1:
				ati.asd  = 1;
				ati.ase  = 0;
				ati.astt = 1;
				break;

			case 2:
				ati.asd  = 1;
				ati.ase  = 1;
				ati.astt = 1;
				break;
		}

		ati.aaf          = SendMessageA(GetDlgItem(wnd,IDCB_AAFILTER),  CB_GETCURSEL, 0, 0);
		ati.mlf          = SendMessageA(GetDlgItem(wnd,IDCB_MLAA),      CB_GETCURSEL, 0, 0);
		ati.aniso_degree = SendMessageA(GetDlgItem(wnd,IDCB_AFMODE), CB_GETCURSEL, 0, 0);
		
		if(ati.aniso_degree != 0)
		{
			ati.aniso_degree = SendMessageA(GetDlgItem(wnd,IDCB_AFLEVEL), CB_GETCURSEL, 0, 0);
			ati.aniso_degree = sel_to_aniso(ati.aniso_degree);
		}

		ati.tfq                         = SendMessageA(GetDlgItem(wnd,IDCB_TFQ),       CB_GETCURSEL, 0, 0);
		ati.surface_format_replacements = SendMessageA(GetDlgItem(wnd,IDCB_SFO),       CB_GETCURSEL, 0, 0);
		ati.vsync_control               = SendMessageA(GetDlgItem(wnd,IDCB_VSYNC),     CB_GETCURSEL, 0, 0);
		ati.triple_buffering            = SendMessageA(GetDlgItem(wnd,IDCB_TBUFFER),   CB_GETCURSEL, 0, 0);
		ati.tessellation_option         = SendMessageA(GetDlgItem(wnd,IDCB_TESSMODE),  CB_GETCURSEL, 0, 0);
		ati.tessellation                = SendMessageA(GetDlgItem(wnd,IDCB_TESSLEVEL), CB_GETCURSEL, 0, 0);

		if(ati.tessellation_option < 2)
		{
			ati.tessellation = 0;
		}
		else
		{
			ati.tessellation_option = 2;
			ati.tessellation = sel_to_tess(ati.tessellation);
		}

		//reg_set_value(key, "Main3D"          , ati.main3d);
		reg_set_value(key, "AntiAlias"       , ati.anti_alias);
		reg_set_value(key, "AntiAliasSamples", ati.anti_alias_samples);
		reg_set_value(key, "MLF"             , ati.mlf);
		reg_set_value(key, "AAF"             , ati.aaf);
		reg_set_value(key, "ASE"             , ati.ase);
		reg_set_value(key, "ASD"             , ati.asd);
		reg_set_value(key, "ASTT"            , ati.astt);

		reg_set_value(key, "AnisoDegree", ati.aniso_degree);
		//reg_set_value(key, "TextureLod" , ati.texture_lod);
		reg_set_value(key, "TFQ"        , ati.tfq);

		reg_set_value(key, "VSyncControl"       , ati.vsync_control);
		reg_set_value(key, "Tessellation"       , ati.tessellation);
		reg_set_value(key, "Tessellation_OPTION", ati.tessellation_option);

		reg_set_value(key, "SurfaceFormatReplacements", ati.surface_format_replacements);
		reg_set_value(key, "EnableTripleBuffering"    , ati.triple_buffering);
	}
	else
	{
		MessageBoxA(NULL, "Coud not save settings to registry.", "Error!", MB_OK);
	}

	return res;
}

LRESULT CALLBACK window_proc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{
		case WM_CREATE:
		{
			HWND item;

			CreateWindowA("button", "OK",     WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 245, 395, 70, 25, wnd, (HMENU)IDOK,     instance, NULL);
			CreateWindowA("button", "Cancel", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 320, 395, 70, 25, wnd, (HMENU)IDCANCEL, instance, NULL);
			CreateWindowA("button", "Apply",  WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 395, 395, 70, 25, wnd, (HMENU)IDAPPLY,  instance, NULL);

			CreateWindowA("button", NULL, WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 3,  -4, 463,  38, wnd, (HMENU)-1, instance, NULL);
			CreateWindowA("button", NULL, WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 3,  29, 463, 138, wnd, (HMENU)-1, instance, NULL);
			CreateWindowA("button", NULL, WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 3, 162, 463, 113, wnd, (HMENU)-1, instance, NULL);
			CreateWindowA("button", NULL, WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 3, 270, 463,  63, wnd, (HMENU)-1, instance, NULL);
			CreateWindowA("button", NULL, WS_VISIBLE | WS_CHILD | BS_GROUPBOX, 3, 328, 463,  63, wnd, (HMENU)-1, instance, NULL);

			CreateWindowA("static", "Preset",                      WS_VISIBLE | WS_CHILD | SS_LEFT, 7,  10, 190, 16, wnd, (HMENU)-1, instance, NULL);
			CreateWindowA("static", "Anti-Aliasing Mode",          WS_VISIBLE | WS_CHILD | SS_LEFT, 7,  42, 190, 16, wnd, (HMENU)-1, instance, NULL);
			CreateWindowA("static", "Anti-Aliasing Samples",       WS_VISIBLE | WS_CHILD | SS_LEFT, 7,  67, 190, 16, wnd, (HMENU)-1, instance, NULL);
			CreateWindowA("static", "Anti-Aliasing Filter",        WS_VISIBLE | WS_CHILD | SS_LEFT, 7,  93, 190, 16, wnd, (HMENU)-1, instance, NULL);
			CreateWindowA("static", "Anti-Aliasing Method",        WS_VISIBLE | WS_CHILD | SS_LEFT, 7, 118, 190, 16, wnd, (HMENU)-1, instance, NULL);
			CreateWindowA("static", "Morphological Filtering",     WS_VISIBLE | WS_CHILD | SS_LEFT, 7, 143, 190, 16, wnd, (HMENU)-1, instance, NULL);
			CreateWindowA("static", "Anisotropic Filtering Mode",  WS_VISIBLE | WS_CHILD | SS_LEFT, 7, 176, 190, 16, wnd, (HMENU)-1, instance, NULL);
			CreateWindowA("static", "Anisotropic Filtering Level", WS_VISIBLE | WS_CHILD | SS_LEFT, 7, 201, 190, 16, wnd, (HMENU)-1, instance, NULL);
			CreateWindowA("static", "Texture Filtering Quality",   WS_VISIBLE | WS_CHILD | SS_LEFT, 7, 226, 190, 16, wnd, (HMENU)-1, instance, NULL);
			CreateWindowA("static", "Surface Format Optimization", WS_VISIBLE | WS_CHILD | SS_LEFT, 7, 251, 190, 16, wnd, (HMENU)-1, instance, NULL);
			CreateWindowA("static", "Wait for Vertical Refresh",   WS_VISIBLE | WS_CHILD | SS_LEFT, 7, 284, 190, 16, wnd, (HMENU)-1, instance, NULL);
			CreateWindowA("static", "OpenGL Triple Buffering",     WS_VISIBLE | WS_CHILD | SS_LEFT, 7, 309, 190, 16, wnd, (HMENU)-1, instance, NULL);
			CreateWindowA("static", "Tessellation Mode",           WS_VISIBLE | WS_CHILD | SS_LEFT, 7, 342, 190, 16, wnd, (HMENU)-1, instance, NULL);
			CreateWindowA("static", "Maximum Tessellation Level",  WS_VISIBLE | WS_CHILD | SS_LEFT, 7, 367, 190, 16, wnd, (HMENU)-1, instance, NULL);

			CreateWindowA("combobox", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 210,   7, 250, 200, wnd, (HMENU)IDCB_PRESET   , instance, NULL);
			CreateWindowA("combobox", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 210,  40, 250, 200, wnd, (HMENU)IDCB_AAMODE   , instance, NULL);
			CreateWindowA("combobox", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 210,  65, 250, 200, wnd, (HMENU)IDCB_AASAMPLE , instance, NULL);
			CreateWindowA("combobox", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 210,  90, 250, 200, wnd, (HMENU)IDCB_AAFILTER , instance, NULL);
			CreateWindowA("combobox", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 210, 115, 250, 200, wnd, (HMENU)IDCB_AAMETHOD , instance, NULL);
			CreateWindowA("combobox", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 210, 140, 250, 200, wnd, (HMENU)IDCB_MLAA     , instance, NULL);
			CreateWindowA("combobox", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 210, 173, 250, 200, wnd, (HMENU)IDCB_AFMODE   , instance, NULL);
			CreateWindowA("combobox", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 210, 198, 250, 200, wnd, (HMENU)IDCB_AFLEVEL  , instance, NULL);
			CreateWindowA("combobox", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 210, 223, 250, 200, wnd, (HMENU)IDCB_TFQ      , instance, NULL);
			CreateWindowA("combobox", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 210, 248, 250, 200, wnd, (HMENU)IDCB_SFO      , instance, NULL);
			CreateWindowA("combobox", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 210, 281, 250, 200, wnd, (HMENU)IDCB_VSYNC    , instance, NULL);
			CreateWindowA("combobox", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 210, 306, 250, 200, wnd, (HMENU)IDCB_TBUFFER  , instance, NULL);
			CreateWindowA("combobox", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 210, 339, 250, 200, wnd, (HMENU)IDCB_TESSMODE , instance, NULL);
			CreateWindowA("combobox", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 210, 364, 250, 200, wnd, (HMENU)IDCB_TESSLEVEL, instance, NULL);

			item = GetDlgItem(wnd, IDCB_PRESET);
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Custom settings");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Optimal performance");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"High performance");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Balanced");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"High quality");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Optimal quality");
			SendMessageA(item, CB_SETCURSEL, 0, 0);
			EnableWindow(item, FALSE); // disabled for now

			item = GetDlgItem(wnd, IDCB_AAMODE);
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Use application settings");
			//SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Enhance application settings");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Override application settings");
			SendMessageA(item, CB_SETCURSEL, ati.anti_alias - 1, 0);

			//shows "Use application setting" if previous is so
			item = GetDlgItem(wnd, IDCB_AASAMPLE);
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"None");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"2x");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"4x");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"8x");
			SendMessageA(item, CB_SETCURSEL, aa_to_sel(ati.anti_alias_samples), 0);
			
			if(ati.anti_alias == 1)
			{
				SendMessageA(item, CB_DELETESTRING, 0, 0);
				SendMessageA(item, CB_INSERTSTRING, 0, (LPARAM)"Use application settings");
				SendMessageA(item, CB_SETCURSEL, 0, 0);
				EnableWindow(item, FALSE);
			}

			item = GetDlgItem(wnd, IDCB_AAFILTER);
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Box");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Narrow");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Wide");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Edge");
			SendMessageA(item, CB_SETCURSEL, ati.aaf, 0);

			item = GetDlgItem(wnd, IDCB_AAMETHOD);
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Multisampling");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Adaptive multisampling");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Supersampling");

			if(ati.ase)
				SendMessageA(item, CB_SETCURSEL, 2, 0);
			else
				SendMessageA(item, CB_SETCURSEL, ati.astt ? 1 : 0, 0);

			item = GetDlgItem(wnd, IDCB_MLAA);
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Off");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"On");
			SendMessageA(item, CB_SETCURSEL, ati.mlf, 0);

			item = GetDlgItem(wnd, IDCB_AFMODE);
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Use application settings");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Override application settings");
			SendMessageA(item, CB_SETCURSEL, ati.aniso_degree > 0, 0);

			//shows "Use application setting" if previous is so
			item = GetDlgItem(wnd, IDCB_AFLEVEL);
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"2x");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"4x");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"8x");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"16x");
			SendMessageA(item, CB_SETCURSEL, aniso_to_sel(ati.aniso_degree), 0);
			
			if(ati.aniso_degree == 0)
			{
				SendMessageA(item, CB_DELETESTRING, 0, 0);
				SendMessageA(item, CB_INSERTSTRING, 0, (LPARAM)"Use application settings");
				SendMessageA(item, CB_SETCURSEL, 0, 0);
				EnableWindow(item, FALSE);
			}

			item = GetDlgItem(wnd, IDCB_TFQ);
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"High Quality");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Standard");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Performance");
			SendMessageA(item, CB_SETCURSEL, ati.tfq, 0);

			item = GetDlgItem(wnd, IDCB_SFO);
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Off");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"On");
			SendMessageA(item, CB_SETCURSEL, ati.surface_format_replacements, 0);

			item = GetDlgItem(wnd, IDCB_VSYNC);
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Always off");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Off, unless application specifies");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"On, unless application specifies");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Always on");
			SendMessageA(item, CB_SETCURSEL, ati.vsync_control, 0);

			item = GetDlgItem(wnd, IDCB_TBUFFER);
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Off");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"On");
			SendMessageA(item, CB_SETCURSEL, ati.triple_buffering, 0);

			item = GetDlgItem(wnd, IDCB_TESSMODE);
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"AMD optimized");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Use application settings");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Override application settings");
			SendMessageA(item, CB_SETCURSEL, ati.tessellation_option, 0);

			//shows "AMD optimized" if previous is so
			item = GetDlgItem(wnd, IDCB_TESSLEVEL);
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"Off");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"2x");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"4x");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"6x");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"8x");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"16x");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"32x");
			SendMessageA(item, CB_ADDSTRING, 0, (LPARAM)"64x");
			SendMessageA(item, CB_SETCURSEL, tess_to_sel(ati.tessellation), 0);

			if(ati.tessellation_option < 2)
			{
				char *text = ati.tessellation == 1? "Use application settings" : "AMD optimized";

				SendMessageA(item, CB_DELETESTRING, 0, 0);
				SendMessageA(item, CB_INSERTSTRING, 0, (LPARAM)text);
				SendMessageA(item, CB_SETCURSEL, 0, 0);
				EnableWindow(item, FALSE);
			}

			ShowWindow(wnd, SW_SHOW);
			return 0;
		}
		
		case WM_COMMAND:
			if(HIWORD(wparam) == CBN_SELCHANGE) switch(LOWORD(wparam))
			{
				static LRESULT aasample;
				static LRESULT aflevel;
				static LRESULT tesslevel;

				case IDCB_PRESET:
				break;

				case IDCB_AAMODE:
				{
					HWND main = GetDlgItem(wnd, IDCB_AAMODE);
					HWND sub  = GetDlgItem(wnd, IDCB_AASAMPLE);

					if(SendMessage(main, CB_GETCURSEL, 0, 0) == 0)
					{
						SendMessageA(sub, CB_DELETESTRING, 0, 0);
						SendMessageA(sub, CB_INSERTSTRING, 0, (LPARAM)"Use application settings");
						SendMessageA(sub, CB_SETCURSEL, 0, 0);
						EnableWindow(sub, FALSE);
					}
					else
					{
						SendMessageA(sub, CB_DELETESTRING, 0, 0);
						SendMessageA(sub, CB_INSERTSTRING, 0, (LPARAM)"None");
						SendMessageA(sub, CB_SETCURSEL, aasample, 0);
						EnableWindow(sub, TRUE);
					}
					break;
				}

				case IDCB_AASAMPLE:
				aasample = SendMessage(GetDlgItem(wnd, IDCB_AASAMPLE), CB_GETCURSEL, 0, 0);
				break;

				case IDCB_AAFILTER:
				break;

				case IDCB_AAMETHOD:
				break;

				case IDCB_MLAA:
				break;

				case IDCB_AFMODE:
				{
					HWND main = GetDlgItem(wnd, IDCB_AFMODE);
					HWND sub  = GetDlgItem(wnd, IDCB_AFLEVEL);

					if(SendMessage(main, CB_GETCURSEL, 0, 0) == 0)
					{
						SendMessageA(sub, CB_DELETESTRING, 0, 0);
						SendMessageA(sub, CB_INSERTSTRING, 0, (LPARAM)"Use application settings");
						SendMessageA(sub, CB_SETCURSEL, 0, 0);
						EnableWindow(sub, FALSE);
					}
					else
					{
						SendMessageA(sub, CB_DELETESTRING, 0, 0);
						SendMessageA(sub, CB_INSERTSTRING, 0, (LPARAM)"2x");
						SendMessageA(sub, CB_SETCURSEL, aflevel, 0);
						EnableWindow(sub, TRUE);
					}
					break;
				}

				case IDCB_AFLEVEL:
				aflevel = SendMessage(GetDlgItem(wnd, IDCB_AFLEVEL), CB_GETCURSEL, 0, 0);
				break;

				case IDCB_TFQ:
				break;

				case IDCB_SFO:
				break;

				case IDCB_VSYNC:
				break;

				case IDCB_TBUFFER:
				break;

				case IDCB_TESSMODE:
				{
					HWND    main = GetDlgItem(wnd, IDCB_TESSMODE);
					HWND    sub  = GetDlgItem(wnd, IDCB_TESSLEVEL);
					LRESULT val  = SendMessage(main, CB_GETCURSEL, 0, 0);

					if(val == 2)
					{
						SendMessageA(sub, CB_DELETESTRING, 0, 0);
						SendMessageA(sub, CB_INSERTSTRING, 0, (LPARAM)"Off");
						SendMessageA(sub, CB_SETCURSEL, tesslevel, 0);
						EnableWindow(sub, TRUE);
					}
					else
					{
						char *text = val ? "Use application settings" : "AMD optimized";

						SendMessageA(sub, CB_DELETESTRING, 0, 0);
						SendMessageA(sub, CB_INSERTSTRING, 0, (LPARAM)text);
						SendMessageA(sub, CB_SETCURSEL, 0, 0);
						EnableWindow(sub, FALSE);
					}
					break;
				}

				case IDCB_TESSLEVEL:
				tesslevel = SendMessage(GetDlgItem(wnd, IDCB_TESSLEVEL), CB_GETCURSEL, 0, 0);
				break;
			}
			else switch(LOWORD(wparam))
			{
				case IDOK:
				save_settings(wnd);
				DestroyWindow(wnd);
				break;

				case IDCANCEL:
				DestroyWindow(wnd);
				break;

				case IDAPPLY:
				save_settings(wnd);
				break;
			}
			break;
			
		case WM_KEYUP:
			if(wparam == VK_ESCAPE)
				DestroyWindow(wnd);
			break;

		case WM_CLOSE:
			DestroyWindow(wnd);
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}
	return DefWindowProcA(wnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, char *cmdline, int cmdshow)
{
	RECT rect = {0, 0, 469, 425};
	WNDCLASS wc;
	MSG msg;
	HWND wnd;

	if(load_settings() != ERROR_SUCCESS)
		return -1;

	instance = inst;

	wc.style         = 0; //CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = window_proc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = inst;
	wc.hIcon         = LoadIcon(inst, MAKEINTRESOURCE(101));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = "ACE";
	
	if(!RegisterClassA(&wc))
		return -2;

	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	wnd = CreateWindowA("ACE", "ACE", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
		NULL, NULL, inst, NULL);

	if(!wnd)
		return -3;

	while(GetMessageA(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return ERROR_SUCCESS;
}