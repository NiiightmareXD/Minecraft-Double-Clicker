#include <Windows.h>
#include <chrono>
#include <thread>
#include <spdlog/spdlog.h>
#include <random>

LRESULT WINAPI mouse_call(int nCode, WPARAM wParam, LPARAM lParam);

void click(bool& enable);

class mouse_hook
{
public:
	static mouse_hook& Instance()
	{
		static mouse_hook mh;
		return mh;
	}
	void uninstall_hook() const;
	void install_hook();
	int msg_loop();
	HHOOK hook{};
	MSG msg{};
};

void mouse_hook::uninstall_hook() const
{
	if (!UnhookWindowsHookEx(hook))
	{
		spdlog::error("FAILED to unhook the windows hook error: {}", GetLastError());
	}
}

void mouse_hook::install_hook()
{
	if (!(hook = SetWindowsHookEx(WH_MOUSE_LL, mouse_call, nullptr, NULL)))
	{
		spdlog::error("FAILED to set the windows hook error: {}", GetLastError());
	}
}

int mouse_hook::msg_loop()
{
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	uninstall_hook();
	return static_cast<int>(msg.wParam);
}

int main()
{
	std::ios::sync_with_stdio(false);
	mouse_hook::Instance().install_hook();
	return mouse_hook::Instance().msg_loop();
}

LRESULT WINAPI mouse_call(const int nCode, const WPARAM wParam, const LPARAM lParam)
{
	static bool enable = true;

	if (enable == true && nCode == 0 && wParam == WM_LBUTTONUP)
	{
		spdlog::info("Left mouse button is up sending Click");
		std::jthread(click, std::ref(enable)).detach();
	}
	return CallNextHookEx(mouse_hook::Instance().hook, nCode, wParam, lParam);
}

int random_in_range(const int min, const int max)
{
	static std::random_device rd;
	static std::mt19937_64 mt(rd());
	const std::uniform_int_distribution uni(min, max);
	return uni(mt);
}

void click(bool& enable)
{
	static INPUT Dinput{};
	static INPUT Uinput{};
	static UINT DuSent;
	static UINT UuSent;

	Dinput.type = INPUT_MOUSE;
	Dinput.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

	Uinput.type = INPUT_MOUSE;
	Uinput.mi.dwFlags = MOUSEEVENTF_LEFTUP;

	std::this_thread::sleep_for(std::chrono::milliseconds(random_in_range(30, 40)));
	enable = false;
	DuSent = SendInput(1, &Dinput, sizeof(INPUT));
	enable = true;
	if (DuSent != 1)
	{
		spdlog::error("Sending input failed error: {}", GetLastError());
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(random_in_range(10, 15)));
	enable = false;
	UuSent = SendInput(1, &Uinput, sizeof(INPUT));
	enable = true;
	if (UuSent != 1)
	{
		spdlog::error("Sending input failed error: {}", GetLastError());
	}
}
