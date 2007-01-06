
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

#include <fstream>
std::ofstream out("foo");

class screen
{
public:
	screen(const obby::buffer& buffer);
	~screen();

	unsigned int get_cursor_x();
	unsigned int get_cursor_y();
//	void set_cursor(unsigned int x, unsigned int y);

	void on_insert(obby::position position, const std::string& text);
	void on_erase(obby::position from, obby::position to);

	void redraw();

protected:
	const obby::buffer& m_buffer;
	unsigned int m_scroll_x, m_scroll_y;
	unsigned int m_cursor;
//	unsigned int m_cursor_x, m_cursor_y;
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

unsigned int screen::get_cursor_x()
{
	unsigned int cursor_x, cursor_y;
	m_buffer.position_to_coord(m_cursor, cursor_x, cursor_y);
	return cursor_x;
}

unsigned int screen::get_cursor_y()
{
	unsigned int cursor_x, cursor_y;
	m_buffer.position_to_coord(m_cursor, cursor_x, cursor_y);
	return cursor_y;
}

void screen::on_insert(obby::position position, const std::string& text)
{
//	obby::position cursor;
//	cursor = m_buffer.coord_to_position(m_cursor_x, m_cursor_y);
	if(position <= m_cursor) m_cursor += text.length();
//	m_buffer.position_to_coord(cursor, m_cursor_x, m_cursor_y);

	redraw();
}

void screen::on_erase(obby::position from, obby::position to)
{
	assert(to > from);

//	obby::position cursor;
//	cursor = m_buffer.coord_to_position(m_cursor_x, m_cursor_y);
	if(m_cursor >= to)
		m_cursor -= (to - from);
	else if(m_cursor >= from && m_cursor < to)
		m_cursor = from;
//	m_buffer.position_to_coord(cursor, m_cursor_x, m_cursor_y);

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
			new_text = new_text.erase(0, COLS);

		printw(new_text.c_str() );
	}

	unsigned int cursor_x, cursor_y;
	m_buffer.position_to_coord(m_cursor, cursor_x, cursor_y);
	out << cursor_x << ", " << cursor_y << std::endl;
	int cx = cursor_x, cy = cursor_y;
	cx -= m_scroll_x, cy -= m_scroll_y;

	assert(cx >= 0 && cy >= 0 && cx < COLS && cy < LINES);
	move(cy, cx);
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
			unsigned int x = m_screen.get_cursor_x();
			unsigned int y = m_screen.get_cursor_y();

			// Check for printable characters
			if(isprint(c) || c == '\n')
			{
				char string[2] = { c, '\0' };
				m_buffer.insert(m_buffer.coord_to_position(x, y), string);
				m_screen.on_insert(m_buffer.coord_to_position(x, y), string);
			}

			if(c == '\r')
			{
				m_buffer.insert(m_buffer.coord_to_position(x, y), "\n");
				m_screen.on_insert(m_buffer.coord_to_position(x, y), "\n");
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
