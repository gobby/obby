
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <sigc++/bind.h>
#include <curses.h>

// net6/socket.hpp overwrites ERR
const int CURSES_ERR = ERR;

#include "record.hpp"
#include "insert_record.hpp"
#include "delete_record.hpp"
#include "buffer.hpp"
#include "client_buffer.hpp"
#include "server_buffer.hpp"
#include "rendezvous.hpp"

#undef ERR
#define ERR CURSES_ERR

class screen
{
public:
	screen(const obby::buffer& buffer);
	~screen();

	unsigned int get_cursor() const;
	unsigned int get_cursor_x() const;
	unsigned int get_cursor_y() const;

	void set_cursor(unsigned int x, unsigned int y);
	void move_cursor(int by_x, int by_y, bool priority_x);

	void on_insert(obby::position position, const std::string& text);
	void on_erase(obby::position from, obby::position to);

	void redraw();
	void scroll_to_cursor();

protected:
	const obby::buffer& m_buffer;
	unsigned int m_scroll_x, m_scroll_y;
	unsigned int m_cursor;
};

screen::screen(const obby::buffer& buffer)
 : m_buffer(buffer), m_scroll_x(0), m_scroll_y(0), m_cursor(0)
{
	initscr();
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
}

screen::~screen()
{
	endwin();
}

unsigned int screen::get_cursor() const
{
	return m_cursor;
}

unsigned int screen::get_cursor_x() const
{
	unsigned int cursor_x, cursor_y;
	m_buffer.position_to_coord(m_cursor, cursor_x, cursor_y);
	return cursor_x;
}

unsigned int screen::get_cursor_y() const
{
	unsigned int cursor_x, cursor_y;
	m_buffer.position_to_coord(m_cursor, cursor_x, cursor_y);
	return cursor_y;
}

void screen::set_cursor(unsigned int x, unsigned int y)
{
	unsigned int cursor_x, cursor_y;
	m_buffer.position_to_coord(m_cursor, cursor_x, cursor_y);
	move_cursor(x - cursor_x, y - cursor_y, false);
}

void screen::move_cursor(int by_x, int by_y, bool priority_x)
{
	unsigned int cx, cy;
	m_buffer.position_to_coord(m_cursor, cx, cy);
	int cursor_x = static_cast<int>(cx) + by_x;
	int cursor_y = static_cast<int>(cy) + by_y;

	if(cursor_y < 0) cursor_y = 0;
	if(cursor_y > m_buffer.get_line_count() - 1)
		cursor_y = m_buffer.get_line_count() - 1;

	if(cursor_x < 0)
	{
		if(priority_x)
		{
			if(cursor_y > 0)
			{
				cursor_y --;
				cursor_x = m_buffer.get_line(cursor_y).length();
			}
			else
			{
				cursor_x = 0;
			}
		}
		else
		{
			cursor_x = 0;
		}
	}

	if(cursor_x > m_buffer.get_line(cursor_y).length() )
	{
		if(priority_x)
		{
			if(cursor_y != m_buffer.get_line_count() - 1)
			{
				cursor_y ++;
				cursor_x = 0;
			}
			else
			{
				cursor_x = m_buffer.get_line(cursor_y).length();
			}
		}
		else
		{
			cursor_x = m_buffer.get_line(cursor_y).length();
		}
	}

	m_cursor = m_buffer.coord_to_position(cursor_x, cursor_y);
	scroll_to_cursor();
	redraw();
}

void screen::on_insert(obby::position position, const std::string& text)
{
	if(position <= m_cursor) m_cursor += text.length();
	scroll_to_cursor();
	redraw();
}

void screen::on_erase(obby::position from, obby::position to)
{
	assert(to >= from);
	if(m_cursor >= to)
		m_cursor -= (to - from);
	else if(m_cursor >= from && m_cursor < to)
		m_cursor = from;
	scroll_to_cursor();
	redraw();
}

void screen::redraw()
{
	unsigned int from_y = m_scroll_y;
	unsigned int to_y = m_scroll_y + LINES;
	if(to_y > m_buffer.get_line_count() ) to_y = m_buffer.get_line_count();

	for(unsigned int y = from_y; y < to_y; ++ y)
	{
		move(y - from_y, 0);
		clrtoeol();
		move(y - from_y, 0);

		std::string new_text = m_buffer.get_line(y);
		if(new_text.length() <= m_scroll_x) continue;

		
		new_text = new_text.substr(m_scroll_x);
		if(new_text.length() > COLS)
			new_text = new_text.substr(0, COLS);

		printw(new_text.c_str() );
	}

	unsigned int cursor_x, cursor_y;
	m_buffer.position_to_coord(m_cursor, cursor_x, cursor_y);
	int cx = cursor_x, cy = cursor_y;
	cx -= m_scroll_x, cy -= m_scroll_y;

	assert(cx >= 0 && cy >= 0 && cx < COLS && cy < LINES);
	move(cy, cx);
}

void screen::scroll_to_cursor()
{
	unsigned int cursor_x, cursor_y;
	m_buffer.position_to_coord(m_cursor, cursor_x, cursor_y);

	if(m_scroll_x + COLS <= cursor_x)
		m_scroll_x = cursor_x - COLS + 1;
	if(cursor_x < m_scroll_x)
		m_scroll_x = cursor_x;

	if(m_scroll_y + LINES < cursor_y)
		m_scroll_y = cursor_y - LINES;
	if(cursor_y < m_scroll_y)
		m_scroll_y = cursor_y;
}

class curses_editor : public sigc::trackable
{
public:
	curses_editor(int argc, char* argv[]);
	~curses_editor();

	void run();
	void quit();

protected:
	void on_join(net6::client::peer& peer);
	void on_part(net6::client::peer& peer);
	void on_close();
	void on_sync();
	void on_login_failed(const std::string& reason);
	void on_insert(const obby::insert_record& record);
	void on_delete(const obby::delete_record& record);

	int m_port;
	bool m_quit;
	bool m_synced;
	obby::client_buffer m_buffer;
	screen m_screen;
};

curses_editor::curses_editor(int argc, char* argv[])
 : m_port(argc > 3 ? strtol(argv[3], NULL, 10) : 6521), m_quit(true),
   m_synced(false), m_buffer(argv[2], m_port), m_screen(m_buffer)
{
	m_buffer.login(argv[1]);

	m_buffer.join_event().connect(
		sigc::mem_fun(*this, &curses_editor::on_join) );
	m_buffer.part_event().connect(
		sigc::mem_fun(*this, &curses_editor::on_part) );
	m_buffer.close_event().connect(
		sigc::mem_fun(*this, &curses_editor::on_close) );
	m_buffer.sync_event().connect(
		sigc::mem_fun(*this, &curses_editor::on_sync) );
	m_buffer.login_failed_event().connect(
		sigc::mem_fun(*this, &curses_editor::on_login_failed) );
	m_buffer.insert_event().connect(
		sigc::mem_fun(*this, &curses_editor::on_insert) );
	m_buffer.delete_event().connect(
		sigc::mem_fun(*this, &curses_editor::on_delete) );
}

curses_editor::~curses_editor()
{
	if(!m_quit)
		quit();
}

void curses_editor::run()
{
	halfdelay(2);
	m_quit = false;
	
	while(!m_quit)
	{
		int c = getch();
		if(c != ERR && m_synced)
		{
			// Get cursor position
//			unsigned int x = m_screen.get_cursor_x();
//			unsigned int y = m_screen.get_cursor_y();

			// Check for printable characters
			if(isprint(c) && c <= 0xff)
			{
				char string[2] = { c, '\0' };
				m_buffer.insert(m_screen.get_cursor(), string);
				m_screen.on_insert(m_screen.get_cursor(), string);
			}

			if(c == '\r')
			{
				m_buffer.insert(m_screen.get_cursor(), "\n");
				m_screen.on_insert(m_screen.get_cursor(), "\n");
			}

			if(c == KEY_LEFT)
				m_screen.move_cursor(-1, 0, true);
			if(c == KEY_RIGHT)
				m_screen.move_cursor(1, 0, true);
			if(c == KEY_UP)
				m_screen.move_cursor(0, -1, false);
			if(c == KEY_DOWN)
				m_screen.move_cursor(0, 1, false);
			if(c == KEY_BACKSPACE)
			{
				m_buffer.erase(m_screen.get_cursor() - 1, m_screen.get_cursor() );
				m_screen.on_erase(m_screen.get_cursor() - 1, m_screen.get_cursor() );
			}

			if(c == 'q')
				quit();
		}

		m_buffer.select(0);
	}
}

void curses_editor::quit()
{
	m_quit = true;
}

void curses_editor::on_join(net6::client::peer& peer)
{
}

void curses_editor::on_part(net6::client::peer& peer)
{
}

void curses_editor::on_close()
{
	quit();
}

void curses_editor::on_sync()
{
	m_screen.on_insert(0, m_buffer.get_whole_buffer() );
	m_synced = true;
}

void curses_editor::on_login_failed(const std::string& reason)
{
	quit();
}

void curses_editor::on_insert(const obby::insert_record& record)
{
	m_screen.on_insert(record.get_position(), record.get_text() );
}

void curses_editor::on_delete(const obby::delete_record& record)
{
	m_screen.on_erase(record.get_begin(), record.get_end() );
}

int main(int argc, char* argv[]) try
{
	if(argc < 3)
	{
		std::cerr << "Usage: " << argv[0] << " user host [port]"
		          << std::endl;
		return 1;
	}
	else
	{
		curses_editor editor(argc, argv);
		editor.run();

		return 0;
	}
}
catch(std::runtime_error& e)
{
	std::cerr << e.what() << std::endl;
	return 1;
}
