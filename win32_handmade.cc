
#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")
#pragma comment(lib, "winmm")
#pragma comment(lib, "d3d11")

#include <windows.h>
#include <stdint.h>

#define internal        static
#define local_persist   static
#define global_variable static

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

struct Win32OffscreenBuffer
{
    BITMAPINFO info;
    void *memory;
    int width;
    int height;
    int pitch;
};

struct Win32WindowDimension
{
    int width;
    int height;
};

global_variable bool global_runing;
global_variable Win32OffscreenBuffer global_back_buffer;

internal Win32WindowDimension win32_get_window_dimension(HWND window)
{
    Win32WindowDimension result;

    RECT client_rect;
    GetClientRect(window, &client_rect);
    result.width = client_rect.right - client_rect.left;
    result.height = client_rect.bottom - client_rect.top;

    return result;
}

internal void render_weird_gradient(Win32OffscreenBuffer buffer, int blue_offset, int green_offset)
{
    u8 *row = (u8 *)buffer.memory;
    for (int y = 0; y < buffer.height; ++y) {
        u32 *pixel = (u32 *)row;

        for (int x = 0; x < buffer.width; ++x) {
            u8 blue = (u8)(x + blue_offset);
            u8 green = (u8)(y + green_offset);

            *pixel++ = ((green << 8) | blue);
        }

        row += buffer.pitch;
    }
}

internal void win32_resize_DIB_section(Win32OffscreenBuffer *buffer, int width, int height)
{
    if (buffer->memory) {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    buffer->width = width;
    buffer->height = height;

    int bytes_per_pixel = 4;

    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    buffer->info.bmiHeader.biHeight = buffer->height;
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;

    int bitmap_memory_size = (buffer->width * buffer->height) * bytes_per_pixel;
    buffer->memory = VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT, PAGE_READWRITE);
    buffer->pitch = width * bytes_per_pixel;
}

internal void win32_display_buffer_in_window(HDC device_context, int window_width, int window_height, Win32OffscreenBuffer buffer)
{
    StretchDIBits(device_context,
                  0, 0, window_width, window_height,
                  0, 0, buffer.width, buffer.height,
                  buffer.memory,
                  &buffer.info,
                  DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK main_window_callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
    LRESULT result = 0;

    switch (message) {
    case WM_CLOSE:
        global_runing = false;
        break;
    case WM_ACTIVATEAPP:
        OutputDebugString("WM_ACTIVATEAPP\n");
        break;
    case WM_DESTROY:
        global_runing = false;
        break;
    case WM_PAINT: {
        PAINTSTRUCT paint;
        HDC device_context = BeginPaint(window, &paint);
        Win32WindowDimension dimension = win32_get_window_dimension(window);
        win32_display_buffer_in_window(device_context, dimension.width, dimension.height, global_back_buffer);
        EndPaint(window, &paint);
        break;
    }
    default:
        result = DefWindowProc(window, message, w_param, l_param);
        break;
    }

    return result;
}

int CALLBACK WinMain(HINSTANCE h_instance, HINSTANCE h_prev_instance, LPSTR lp_cmd_line, int n_show_cmd)
{
    WNDCLASS windows_class = {};

    win32_resize_DIB_section(&global_back_buffer, 1280, 720);

    windows_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windows_class.lpfnWndProc = main_window_callback;
    windows_class.hInstance = h_instance;
    windows_class.lpszClassName = "Handmade Hero";

    if (RegisterClassA(&windows_class)) {
        HWND window = CreateWindowExA(0,
                                      windows_class.lpszClassName,
                                      "Handmade hero",
                                      WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      NULL,
                                      NULL,
                                      h_instance,
                                      NULL);

        if (window) {
            HDC device_context = GetDC(window);

            int x_offset = 0;
            int y_offset = 0;

            global_runing = true;
            while (global_runing) {
                MSG message;
                while (PeekMessage(&message, NULL, NULL, NULL, PM_REMOVE)) {
                    if (message.message == WM_QUIT) {
                        global_runing = false;
                    }

                    TranslateMessage(&message);
                    DispatchMessageA(&message);
                }

                render_weird_gradient(global_back_buffer, x_offset, y_offset);

                Win32WindowDimension dimension = win32_get_window_dimension(window);
                win32_display_buffer_in_window(device_context, dimension.width, dimension.height, global_back_buffer);

                ++x_offset;
                y_offset += 2;
            }
        } else {
            // TODO Logging
        }

    } else {
        // TODO Logging
    }

    return 0;
}
