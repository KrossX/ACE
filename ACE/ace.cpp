/* Copyright (c) 2015 KrossX <krossx@live.com>
* License: http://www.opensource.org/licenses/mit-license.html  MIT License
*/

#include "ace.h"
#include "resource.h"
#include "commctrl.h"

#include <string>

HKEY display_reg_read;
HKEY display_reg_write;

struct
{
	int main3d;
	int anti_alias, anti_alias_samples, aaf, mlf;
	int aniso_type, aniso_degree;
	int texture_lod;
	int tfq;
	int vsync_control;
	int tessellation, tessellation_option;
	int surface_format_replacements;
	int triple_buffering;
} values;


wchar_t text_presets[][64] = { L"Custom settings", L"Optimal performance", L"High performance", L"Balanced", L"High quality", L"Optimal quality"};
wchar_t text_antialias[][64] = { L"Application-controlled", L"2x", L"4x", L"8x"};
wchar_t text_aniso[][64] = { L"Application-controlled", L"2x", L"4x", L"8x", L"16x"};
wchar_t text_mipmap[][64] = { L"High performance", L"Performance", L"Quality", L"High quality"};
wchar_t text_texture[][64] = { L"Performance", L"Standard", L"High quality"};
wchar_t text_vsync[][64] = { L"Always off", L"Default off", L"Default on", L"Always on"};
wchar_t text_tess[][64] = { L"AMD optimized", L"Application-controlled", L"Off", L"2x", L"4x", L"6x", L"8x", L"16x", L"32x", L"64x" };

void set_preset(HWND hwnd, int value)
{
	values.main3d = value;


}

void set_starting_values(HWND hwnd)
{
	SendMessage(GetDlgItem(hwnd, IDC_SLIDER_PRESETS), TBM_SETPOSNOTIFY, 0, values.main3d);

	if (values.anti_alias == 1)
		SendMessage(GetDlgItem(hwnd, IDC_SLIDER_ANTIALIAS), TBM_SETPOSNOTIFY, 0, 0);
	else
		SendMessage(GetDlgItem(hwnd, IDC_SLIDER_ANTIALIAS), TBM_SETPOSNOTIFY, 0, (LPARAM)(sqrt(values.anti_alias_samples)+0.5));

	SendMessage(GetDlgItem(hwnd, IDC_COMBO_AAMODE), CB_SETCURSEL, 0, 0);
	SendMessage(GetDlgItem(hwnd, IDC_COMBO_AAFILTER), CB_SETCURSEL, values.aaf, 0);

	CheckDlgButton(hwnd, IDC_CHECK_MLAA, values.mlf ? BST_CHECKED : BST_UNCHECKED);

	if (values.aniso_degree == 0)
		SendMessage(GetDlgItem(hwnd, IDC_SLIDER_ANISO), TBM_SETPOSNOTIFY, 0, 0);
	else
		SendMessage(GetDlgItem(hwnd, IDC_SLIDER_ANISO), TBM_SETPOSNOTIFY, 0, (LPARAM)(sqrt(values.aniso_degree) + 0.5));

	SendMessage(GetDlgItem(hwnd, IDC_SLIDER_MIPMAP), TBM_SETPOSNOTIFY, 0, (LPARAM)(3 - values.texture_lod));
	SendMessage(GetDlgItem(hwnd, IDC_SLIDER_TEXTURE), TBM_SETPOSNOTIFY, 0, (LPARAM)(2 - values.tfq));

	SendMessage(GetDlgItem(hwnd, IDC_SLIDER_VSYNC), TBM_SETPOSNOTIFY, 0, (LPARAM)(values.vsync_control));

	if (values.tessellation_option < 2)
	{
		SendMessage(GetDlgItem(hwnd, IDC_SLIDER_TESS), TBM_SETPOSNOTIFY, 0, (LPARAM)(values.tessellation_option));
	}
	else
	{
		switch (values.tessellation)
		{
		case 1: SendMessage(GetDlgItem(hwnd, IDC_SLIDER_TESS), TBM_SETPOSNOTIFY, 0, 2); break;
		case 2: SendMessage(GetDlgItem(hwnd, IDC_SLIDER_TESS), TBM_SETPOSNOTIFY, 0, 3); break;
		case 4: SendMessage(GetDlgItem(hwnd, IDC_SLIDER_TESS), TBM_SETPOSNOTIFY, 0, 4); break;
		case 6: SendMessage(GetDlgItem(hwnd, IDC_SLIDER_TESS), TBM_SETPOSNOTIFY, 0, 5); break;
		case 8: SendMessage(GetDlgItem(hwnd, IDC_SLIDER_TESS), TBM_SETPOSNOTIFY, 0, 6); break;
		case 16: SendMessage(GetDlgItem(hwnd, IDC_SLIDER_TESS), TBM_SETPOSNOTIFY, 0, 7); break;
		case 32: SendMessage(GetDlgItem(hwnd, IDC_SLIDER_TESS), TBM_SETPOSNOTIFY, 0, 8); break;
		case 64: SendMessage(GetDlgItem(hwnd, IDC_SLIDER_TESS), TBM_SETPOSNOTIFY, 0, 9); break;
		}
	}

	CheckDlgButton(hwnd, IDC_CHECK_SURFACE, values.surface_format_replacements ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hwnd, IDC_CHECK_TRIPLEBUFFER, values.triple_buffering ? BST_CHECKED : BST_UNCHECKED);
}

void save_settings();

INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		HICON h_icon;

		h_icon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ACEICON), IMAGE_ICON, 32, 32, 0);
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)h_icon);

		h_icon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ACEICON), IMAGE_ICON, 16, 16, 0);
		SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)h_icon);


		SendMessage(GetDlgItem(hwnd, IDC_COMBO_AAMODE), CB_ADDSTRING, 0, (LPARAM)L"Multisampling");
		SendMessage(GetDlgItem(hwnd, IDC_COMBO_AAMODE), CB_ADDSTRING, 0, (LPARAM)L"Adaptive multisampling");
		SendMessage(GetDlgItem(hwnd, IDC_COMBO_AAMODE), CB_ADDSTRING, 0, (LPARAM)L"Supersampling");

		SendMessage(GetDlgItem(hwnd, IDC_COMBO_AAFILTER), CB_ADDSTRING, 0, (LPARAM)L"Box");
		SendMessage(GetDlgItem(hwnd, IDC_COMBO_AAFILTER), CB_ADDSTRING, 0, (LPARAM)L"Narrow-tent");
		SendMessage(GetDlgItem(hwnd, IDC_COMBO_AAFILTER), CB_ADDSTRING, 0, (LPARAM)L"Wide-tent");
		SendMessage(GetDlgItem(hwnd, IDC_COMBO_AAFILTER), CB_ADDSTRING, 0, (LPARAM)L"Edge detect");

		SendMessage(GetDlgItem(hwnd, IDC_SLIDER_PRESETS), TBM_SETRANGE, TRUE, MAKELONG(0, 5));
		SendMessage(GetDlgItem(hwnd, IDC_SLIDER_PRESETS), TBM_SETTICFREQ, 1, 0);

		SendMessage(GetDlgItem(hwnd, IDC_SLIDER_ANTIALIAS), TBM_SETRANGE, TRUE, MAKELONG(0, 3));
		SendMessage(GetDlgItem(hwnd, IDC_SLIDER_ANTIALIAS), TBM_SETTICFREQ, 1, 0);

		SendMessage(GetDlgItem(hwnd, IDC_SLIDER_ANISO), TBM_SETRANGE, TRUE, MAKELONG(0, 4));
		SendMessage(GetDlgItem(hwnd, IDC_SLIDER_ANISO), TBM_SETTICFREQ, 1, 0);

		SendMessage(GetDlgItem(hwnd, IDC_SLIDER_MIPMAP), TBM_SETRANGE, TRUE, MAKELONG(0, 3));
		SendMessage(GetDlgItem(hwnd, IDC_SLIDER_MIPMAP), TBM_SETTICFREQ, 1, 0);

		SendMessage(GetDlgItem(hwnd, IDC_SLIDER_TEXTURE), TBM_SETRANGE, TRUE, MAKELONG(0, 2));
		SendMessage(GetDlgItem(hwnd, IDC_SLIDER_TEXTURE), TBM_SETTICFREQ, 1, 0);

		SendMessage(GetDlgItem(hwnd, IDC_SLIDER_VSYNC), TBM_SETRANGE, TRUE, MAKELONG(0, 3));
		SendMessage(GetDlgItem(hwnd, IDC_SLIDER_VSYNC), TBM_SETTICFREQ, 1, 0);

		SendMessage(GetDlgItem(hwnd, IDC_SLIDER_TESS), TBM_SETRANGE, TRUE, MAKELONG(0, 9));
		SendMessage(GetDlgItem(hwnd, IDC_SLIDER_TESS), TBM_SETTICFREQ, 1, 0);

		set_starting_values(hwnd);
	}
	break;

	case WM_HSCROLL:
	{
		if ((HWND)lparam == GetDlgItem(hwnd, IDC_SLIDER_PRESETS))
		{
			LRESULT value = SendMessage(GetDlgItem(hwnd, IDC_SLIDER_PRESETS), TBM_GETPOS, 0, 0);
			set_preset(hwnd, value);
			SetDlgItemText(hwnd, IDC_LABEL_PRESET, text_presets[value]);
		}
		else  if ((HWND)lparam == GetDlgItem(hwnd, IDC_SLIDER_ANTIALIAS))
		{
			LRESULT value = SendMessage(GetDlgItem(hwnd, IDC_SLIDER_ANTIALIAS), TBM_GETPOS, 0, 0);

			if (value == 0)
			{
				values.anti_alias = 1;
				values.anti_alias_samples = 0;
			}
			else
			{
				values.anti_alias = 2;
				values.anti_alias_samples = (int)pow(2, value);
			}

			SetDlgItemText(hwnd, IDC_LABEL_ANTIALIAS, text_antialias[value]);
		}
		else  if ((HWND)lparam == GetDlgItem(hwnd, IDC_SLIDER_ANISO))
		{
			LRESULT value = SendMessage(GetDlgItem(hwnd, IDC_SLIDER_ANISO), TBM_GETPOS, 0, 0);

			if (value == 0)
				values.aniso_degree = 0;
			else
				values.aniso_degree = (int)pow(2, value);

			SetDlgItemText(hwnd, IDC_LABEL_ANISO, text_aniso[value]);
		}
		else  if ((HWND)lparam == GetDlgItem(hwnd, IDC_SLIDER_MIPMAP))
		{
			LRESULT value = SendMessage(GetDlgItem(hwnd, IDC_SLIDER_MIPMAP), TBM_GETPOS, 0, 0);
			values.texture_lod = 3 - value;
			SetDlgItemText(hwnd, IDC_LABEL_MIPMAP, text_mipmap[value]);
		}
		else  if ((HWND)lparam == GetDlgItem(hwnd, IDC_SLIDER_TEXTURE))
		{
			LRESULT value = SendMessage(GetDlgItem(hwnd, IDC_SLIDER_TEXTURE), TBM_GETPOS, 0, 0);
			values.tfq = 2 - value;
			SetDlgItemText(hwnd, IDC_LABEL_TEXTURE, text_texture[value]);
		}
		else  if ((HWND)lparam == GetDlgItem(hwnd, IDC_SLIDER_VSYNC))
		{
			LRESULT value = SendMessage(GetDlgItem(hwnd, IDC_SLIDER_VSYNC), TBM_GETPOS, 0, 0);
			values.vsync_control = value;
			SetDlgItemText(hwnd, IDC_LABEL_VSYNC, text_vsync[value]);
		}
		else  if ((HWND)lparam == GetDlgItem(hwnd, IDC_SLIDER_TESS))
		{
			LRESULT value = SendMessage(GetDlgItem(hwnd, IDC_SLIDER_TESS), TBM_GETPOS, 0, 0);

			if (value < 2)
			{
				values.tessellation_option = value;
				values.tessellation = 1;
			}
			else
			{
				values.tessellation_option = 2;

				switch (value)
				{
				case 2: values.tessellation = 1; break;
				case 3: values.tessellation = 2; break;
				case 4: values.tessellation = 4; break;
				case 5: values.tessellation = 6; break;
				case 6: values.tessellation = 8; break;
				case 7: values.tessellation = 16; break;
				case 8: values.tessellation = 32; break;
				case 9: values.tessellation = 64; break;
				}
			}

			SetDlgItemText(hwnd, IDC_LABEL_TESS, text_tess[value]);
		}
	}
	break;

	case WM_COMMAND:
	{
		switch (LOWORD(wparam))
		{
		case IDC_COMBO_AAMODE: //if (HIWORD(wparam) == CBN_SELCHANGE)
			break;

		case IDC_COMBO_AAFILTER: if (HIWORD(wparam) == CBN_SELCHANGE)
			values.aaf = SendMessage(GetDlgItem(hwnd, IDC_COMBO_AAFILTER), CB_GETCURSEL, 0, 0);
			break;

		case IDC_CHECK_MLAA:
			values.mlf = IsDlgButtonChecked(hwnd, IDC_CHECK_MLAA);
			break;

		case IDC_CHECK_SURFACE:
			values.surface_format_replacements = IsDlgButtonChecked(hwnd, IDC_CHECK_SURFACE);
			break;

		case IDC_CHECK_TRIPLEBUFFER:
			values.triple_buffering = IsDlgButtonChecked(hwnd, IDC_CHECK_TRIPLEBUFFER);
			break;

		case IDOK:
			save_settings();
			EndDialog(hwnd, ERROR_SUCCESS);
			return TRUE;

		case IDCANCEL:
			EndDialog(hwnd, ERROR_SUCCESS);
			return TRUE;

		case IDAPPLY:
			save_settings();
			return TRUE;
		}
	}
	break;
	}

	return FALSE;
}

int reg_get_value(std::wstring key, int def = 0)
{
	wchar_t buffer[64];
	memset(buffer, 0x00, 64);

	DWORD size = 64;
	LSTATUS result = RegQueryValueExW(display_reg_read, key.c_str(), NULL, NULL, (BYTE*)buffer, &size);

	if (result == ERROR_SUCCESS)
		def = std::stoi(std::wstring(buffer), nullptr, 10);

	return def;
}

void reg_set_value(std::wstring key, int value)
{
	std::wstring v = std::to_wstring(value);
	RegSetValueExW(display_reg_write, key.c_str(), NULL, REG_SZ, (BYTE*)v.c_str(), v.length() * sizeof(wchar_t));
}

void save_settings()
{
	reg_set_value(L"Main3D", values.main3d);
	reg_set_value(L"AntiAlias", values.anti_alias);
	reg_set_value(L"AntiAliasSamples", values.anti_alias_samples);
	reg_set_value(L"MLF", values.mlf);
	reg_set_value(L"AAF", values.aaf);

	reg_set_value(L"AnisoDegree", values.aniso_degree);
	reg_set_value(L"TextureLod", values.texture_lod);
	reg_set_value(L"TFQ", values.tfq);

	reg_set_value(L"VSyncControl", values.vsync_control);
	reg_set_value(L"Tessellation", values.tessellation);
	reg_set_value(L"Tessellation_OPTION", values.tessellation_option);

	reg_set_value(L"SurfaceFormatReplacements", values.surface_format_replacements);
	reg_set_value(L"EnableTripleBuffering", values.triple_buffering);
}

void load_settings()
{
	values.main3d = reg_get_value(L"Main3D");
	values.anti_alias = reg_get_value(L"AntiAlias");
	values.anti_alias_samples = reg_get_value(L"AntiAliasSamples");
	values.mlf = reg_get_value(L"MLF");
	values.aaf = reg_get_value(L"AAF");

	values.aniso_degree = reg_get_value(L"AnisoDegree");
	values.texture_lod = reg_get_value(L"TextureLod");
	values.tfq = reg_get_value(L"TFQ");

	values.vsync_control = reg_get_value(L"VSyncControl");
	values.tessellation = reg_get_value(L"Tessellation");
	values.tessellation_option = reg_get_value(L"Tessellation_OPTION");

	values.surface_format_replacements = reg_get_value(L"SurfaceFormatReplacements");
	values.triple_buffering = reg_get_value(L"EnableTripleBuffering");
}

int CALLBACK wWinMain(HINSTANCE h_instance, HINSTANCE h_prev_instance, LPWSTR cmdline, int ncmd)
{
	wchar_t reg_location[] = L"SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}\\0000\\UMD";
	
	RegOpenKeyExW(HKEY_LOCAL_MACHINE, reg_location, 0, KEY_WRITE, &display_reg_write);
	
	LONG result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, reg_location, 0, KEY_READ, &display_reg_read);
	if (result == ERROR_SUCCESS) load_settings();

	DialogBoxParamW(h_instance, MAKEINTRESOURCE(IDD_ACE), NULL, dialog_proc, 0);

	return ERROR_SUCCESS;
}