#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <sigc++/bind.h>
#include <curses.h>
#include <time.h>

// net6/socket.hpp overwrites ERR
const int CURSES_ERR = ERR;

#include "record.hpp"
#include "insert_record.hpp"
#include "delete_record.hpp"
#include "buffer.hpp"
#include "client_buffer.hpp"
#include "server_buffer.hpp"

#undef ERR
#define ERR CURSES_ERR

class screen
{
public:
	screen(const obby::document& document);
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
	const obby::document& m_document;
	unsigned int m_scroll_x, m_scroll_y;
	unsigned int m_cursor;
};

screen::screen(const obby::document& document)
 : m_document(document), m_scroll_x(0), m_scroll_y(0), m_cursor(0)
{
	initscr();
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	halfdelay(2);
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
	m_document.position_to_coord(m_cursor, cursor_y, cursor_x);
	return cursor_x;
}

unsigned int screen::get_cursor_y() const
{
	unsigned int cursor_x, cursor_y;
	m_document.position_to_coord(m_cursor, cursor_y, cursor_x);
	return cursor_y;
}

void screen::set_cursor(unsigned int x, unsigned int y)
{
	unsigned int cursor_x, cursor_y;
	m_document.position_to_coord(m_cursor, cursor_y, cursor_x);
	move_cursor(x - cursor_x, y - cursor_y, false);
}

void screen::move_cursor(int by_x, int by_y, bool priority_x)
{
	unsigned int cx, cy;
	m_document.position_to_coord(m_cursor, cy, cx);
	int cursor_x = static_cast<int>(cx) + by_x;
	int cursor_y = static_cast<int>(cy) + by_y;

	if(cursor_y < 0) cursor_y = 0;
	if(cursor_y > m_document.get_line_count() - 1)
		cursor_y = m_document.get_line_count() - 1;

	if(cursor_x < 0)
	{
		if(priority_x)
		{
			if(cursor_y > 0)
			{
				cursor_y --;
				cursor_x = m_document.get_line(cursor_y).length();
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

	if(cursor_x > m_document.get_line(cursor_y).length() )
	{
		if(priority_x)
		{
			if(cursor_y != m_document.get_line_count() - 1)
			{
				cursor_y ++;
				cursor_x = 0;
			}
			else
			{
				cursor_x = m_document.get_line(cursor_y).length();
			}
		}
		else
		{
			cursor_x = m_document.get_line(cursor_y).length();
		}
	}

	m_cursor = m_document.coord_to_position(cursor_y, cursor_x);
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
	if(to_y > m_document.get_line_count() ) to_y = m_document.get_line_count();

	for(unsigned int y = from_y; y < to_y; ++ y)
	{
		move(y - from_y, 0);
		clrtoeol();
		move(y - from_y, 0);

		std::string new_text = m_document.get_line(y);
		if(new_text.length() <= m_scroll_x) continue;

		
		new_text = new_text.substr(m_scroll_x);
		if(new_text.length() > COLS)
			new_text = new_text.substr(0, COLS);

		printw("%s", new_text.c_str() );
	}

	unsigned int cursor_x, cursor_y;
	m_document.position_to_coord(m_cursor, cursor_y, cursor_x);
	int cx = cursor_x, cy = cursor_y;
	cx -= m_scroll_x, cy -= m_scroll_y;

	assert(cx >= 0 && cy >= 0 && cx < COLS && cy < LINES);
	move(cy, cx);
}

void screen::scroll_to_cursor()
{
	unsigned int cursor_x, cursor_y;
	m_document.position_to_coord(m_cursor, cursor_y, cursor_x);

	if(m_scroll_x + COLS <= cursor_x)
		m_scroll_x = cursor_x - COLS + 1;
	if(cursor_x < m_scroll_x)
		m_scroll_x = cursor_x;

	if(m_scroll_y + LINES <= cursor_y)
		m_scroll_y = cursor_y - LINES + 1;
	if(cursor_y < m_scroll_y)
		m_scroll_y = cursor_y;
}

class curses_editor : public sigc::trackable
{
public:
	curses_editor(int argc, char* argv[]);
	~curses_editor();

	void dispatch_key(int c);

	bool run();
	void quit();

	const std::string& get_reason() const;

protected:
	void on_join(obby::user& user);
	void on_part(obby::user& user);
	void on_close();
	void on_sync();
	void on_login_failed(const std::string& reason);

	void on_document_insert(obby::document& doc);
	void on_document_remove(obby::document& doc);

	void on_insert(const obby::insert_record& record, obby::document& doc);
	void on_delete(const obby::delete_record& record, obby::document& doc);

	int m_port;
	bool m_quit;
	std::string m_reason;
	bool m_synced;
	obby::client_buffer m_buffer;
	screen* m_screen;
	obby::document* m_document;
};

curses_editor::curses_editor(int argc, char* argv[])
 : m_port(argc > 3 ? strtol(argv[3], NULL, 10) : 6522), m_quit(true),
   m_synced(false), m_buffer(argv[2], m_port), m_screen(NULL)
{
	srand( time(NULL) );
	unsigned int red = rand() % 256;
	unsigned int green = rand() % 256;
	unsigned int blue = rand() % 256;
	
	m_buffer.login_failed_event().connect(
		sigc::mem_fun(*this, &curses_editor::on_login_failed) );
	m_buffer.login(argv[1], red, green, blue);

	m_buffer.user_join_event().connect(
		sigc::mem_fun(*this, &curses_editor::on_join) );
	m_buffer.user_part_event().connect(
		sigc::mem_fun(*this, &curses_editor::on_part) );
	m_buffer.close_event().connect(
		sigc::mem_fun(*this, &curses_editor::on_close) );
	m_buffer.sync_event().connect(
		sigc::mem_fun(*this, &curses_editor::on_sync) );

	m_buffer.insert_document_event().connect(
		sigc::mem_fun(*this, &curses_editor::on_document_insert) );
	m_buffer.remove_document_event().connect(
		sigc::mem_fun(*this, &curses_editor::on_document_remove) );
}

curses_editor::~curses_editor()
{
	if(m_screen)
		delete m_screen;
	if(!m_quit)
		quit();
}

void curses_editor::dispatch_key(int c)
{
	unsigned int first;
	unsigned int last;

	switch(c)
	{
	case KEY_LEFT:
		m_screen->move_cursor(-1, 0, true);
		break;
	case KEY_RIGHT:
		m_screen->move_cursor(1, 0, true);
		break;
	case KEY_UP:
		m_screen->move_cursor(0, -1, false);
		break;
	case KEY_DOWN:
		m_screen->move_cursor(0, 1, false);
		break;
	case KEY_BACKSPACE:
		last = m_screen->get_cursor();
		first = last == 0 ? 0 : m_screen->get_cursor() - 1;
		m_document->erase(first, last);
		m_screen->on_erase(first, last);
		break;
	case KEY_DC:
		first = m_screen->get_cursor();
		last = first == m_document->position_eob() ? first :
			m_screen->get_cursor() + 1;
		m_document->erase(first, last);
		m_screen->on_erase(first, last);
		break;
	case KEY_F(10):
		quit();
		break;
	}
}

bool curses_editor::run()
{
	m_quit = false;
	
	while(!m_quit)
	{
		int c = ERR;
		if(m_screen) c = getch();

		if(c != ERR && m_synced && m_screen)
		{
			// Get cursor position
//			unsigned int x = m_screen.get_cursor_x();
//			unsigned int y = m_screen.get_cursor_y();

			// Check for printable characters
			if(isprint(c) && c <= 0xff)
			{
				char string[2] = { c, '\0' };
				m_document->insert(m_screen->get_cursor(), string);
				m_screen->on_insert(m_screen->get_cursor(), string);
			}

			if(c == '\r')
			{
				m_document->insert(m_screen->get_cursor(), "\n");
				m_screen->on_insert(m_screen->get_cursor(), "\n");
			}

			dispatch_key(c);
		}

		m_buffer.select(0);
	}

	return m_reason.empty();
}

void curses_editor::quit()
{
	m_quit = true;
}

const std::string& curses_editor::get_reason() const
{
	return m_reason;
}

void curses_editor::on_join(obby::user& user)
{
}

void curses_editor::on_part(obby::user& user)
{
}

void curses_editor::on_close()
{
	quit();
}

void curses_editor::on_sync()
{
	obby::buffer::document_iterator iter;
	for(iter = m_buffer.document_begin();
	    iter != m_buffer.document_end();
	    ++ iter)
	{
		on_document_insert(*iter);
/*		m_screen = new screen(*iter);
		m_document = &(*iter);*/
	}

	m_screen->on_insert(0, m_document->get_whole_buffer() );
	m_synced = true;
}

void curses_editor::on_login_failed(const std::string& reason)
{
	m_reason = reason;
	quit();
}

void curses_editor::on_document_insert(obby::document& document)
{
	document.insert_event().after().connect(sigc::bind(
		sigc::mem_fun(*this, &curses_editor::on_insert),
		sigc::ref(document)
	));

	document.delete_event().after().connect(sigc::bind(
		sigc::mem_fun(*this, &curses_editor::on_delete),
		sigc::ref(document)
	));

	m_screen = new screen(document);
	m_document = &document;
}

void curses_editor::on_document_remove(obby::document& document)
{
}

void curses_editor::on_insert(const obby::insert_record& record,
                              obby::document& document)
{
	m_screen->on_insert(record.get_position(), record.get_text() );
}

void curses_editor::on_delete(const obby::delete_record& record,
                              obby::document& document)
{
	m_screen->on_erase(record.get_begin(), record.get_end() );
}

int main(int argc, char* argv[]) try
{
	std::string reason;
	
	if(argc < 3)
	{
		std::cerr << "Usage: " << argv[0] << " user host [port]"
		          << std::endl;
		return 1;
	}
	else
	{
		curses_editor editor(argc, argv);
		if(!editor.run() )
			reason = editor.get_reason();
	}

	if(!reason.empty() )
		std::cerr << "Login failed: " << reason << std::endl;

	return 0;
}
catch(std::runtime_error& e)
{
	std::cerr << e.what() << std::endl;
	return 1;
}
