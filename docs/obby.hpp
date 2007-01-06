// <-- Insert GPL here -->
namespace obby
{

// Idee:
// Es gibt verschiedene "Revisionen" des Dokuments. Wenn der Server einem
// Client eine Aenderung schickt, gibt er zusaetzlich an, was fuer eine
// Revision diese Version des Dokuments nun hat.
// Wenn ein Client eine Aenderung einreicht, gibt er an, an welcher Revision
// diese Aenderung gemacht wurde. Empfaengt der Server eine Aenderung, so
// stellt er aus einer Undo-History die Revision des Clients her, fuegt
// die neue Aenderung ein, und rekonstruiert die temporaer
// rueckgaengig gemachten Aenderungen wieder (evtl. um einige Zeilen/Spalten
// verschoben, wenn die Aenderung des Clients vorher im Dokument war). Dabei
// werden die Koordinaten dieser neuen Aenderung wieder angepasst (wenn
// Aenderungen vor der neuen Aenderung rekonstuiert wurden) und das an alle
// Clients geschickt. Dabei wird eine neue Revision angegeben. Zusaetzlich
// wird dem Client, der diese Aenderung eingereicht hat, gesagt, dass die
// Aenderung nun gesynct wurde.
// Empfaengt ein Client eine Aenderung, so macht er alle noch nicht gesyncten
// Aenderungen von sich selbst temporaer rueckgaengig, fuegt die Aenderung,
// die er empfangen hat ein, und rekonstruiert dann die nicht gesyncten
// Aenderungen, genauso wie es auch der Server macht. Dazu ein kleines Beispiel:
//
// Wir haben folgende Zeile:
// -> obby
// Diese Zeile ist Revision 0.
// Client a fuegt nun an Position 0 ein 'H' ein, sodass er folgendes sieht:
// -> Hobby
// Client b fuegt zur gleichen Zeit ein 'L' ein, sodass er sieht:
// -> Lobby
// Der Server empfaengt die Aenderung von Client a zufaellig zuerst.
// Er merkt, dass seine Revision gleich der von Client a ist, er also nichts
// rueckgaengig zu machen braucht. Er fuegt die Aenderung ein und hat bei sich
// im Dokument:
// -> Hobby
// Den beiden Clients wird jetzt 'Fuegt ein H an Position 0 ein' geschickt, mit
// dem Verweis, dass das, was daraus resultiert, Revision 1 ist.
// Jetzt kommt die Aenderung von Client b an.
// Der Server merkt, dass die fuer Revision 0 bestimmt war. Er macht die
// Aenderung von Client a wieder rueckgaengig, sodass er
// -> obby
// hat, was Revision 0 entspricht. Nun fuegt er die Aenderung von Client b
// ein.
// -> Lobby
// Daraufhin rekonstruiert er die Aenderungen, die er rueckgaengig gemacht hat.
// Das H wird eine Spalte nach rechts verschoben, weil das L vor (bzw. gleich)
// dem H stand. Es ergibt sich demnach:
// -> LHobby
// Es wird also der Vermerk, an Position 0 ein 'L' einzufuegen, an die beiden
// Clients geschickt. Das wird Revision 2 sein.
// Client a, der momentan noch Hobby sieht, empfaengt nun die Aenderung, an
// Position 0 ein H einzufuegen, mit dem Vermerk, dass seine Aenderung damit
// gesynct ist.
// Er macht seine nicht-gesyncte H-Aenderung also rueckgaengig und fuegt es
// gleich darauf wieder ein. Die Aenderung wird aus der Liste an nicht-
// gesyncten Aenderungen entfernt. Das ist noetig, da Client a in Zwischen-
// zeit ja noch haette weitere Zeichen einfuegen koennen, die dann auch
// erst haetten entfernt werden muessen, um das 'H' an der richtigen
// Stelle einzufuegen.
// Client b, bei dem
// -> Lobby
// steht, enthaelt auch diese Aenderung. Er macht seine noch nicht gesyncte L-
// Aenderung also rueckgaengig, fuegt das H ein, und ergaenzt das L dann wieder.
// Dadurch ergibt sich:
// -> HLobby
// Bei Client a kommt jetzt Revision 2 vom Server an. Client a hat keine un-
// gesyncten Aenderungen mehr, die er rueckgaengig machen muesste, fuegt also
// einfach das L an Position 0 ein.
// -> LHobby
// Bei Client b wird es jetzt interessant. Er empfaengt ebenfalls das L, und
// den Vermerk, dass seine Aenderung gesynct ist. Er loescht sein ungesynctes L
// also aus dem Dokument und fuegt es, wie vom Server gesandt, an Position 0
// wieder ein. Dadurch erhaelt auch er
// -> LHobby
// Und siehe da: Alle 3 (Client a, Client b, Server) sind synchron. :-D

#include <list>
#include <string>
#include <vector>
#include <sigc++/signal.hpp>
#include <net6/client.hpp>
#include <net6/server.hpp>
	
class buffer; // text buffer

// position represents a position in the text buffer
class position
{
public:
	position(unsigned int line, unsigned int col);
	position(const position& other);
	~position();
	
	position& operator=(const position& other);

	unsigned int get_line() const;
	unsigned int get_col() const;
private:
	unsigned int m_line;
	unsigned int m_col;
};

// record is a change in the document
class record
{
public:
	record(unsigned int revision, unsigned int from);
	record(unsigned int revision, unsigned int from, unsigned int id);
	~record();

	// applies the record to a text buffer
	virtual void apply(buffer& buf) = 0;
	// checks if this record is valid
	virtual bool is_valid() const = 0;

	unsigned int get_id() const;
	unsigned int get_revision() const;

	// Text has been inserted, apply record coordinates
	virtual void on_insert(const position& pos,
	                       const std::string& text) = 0;
	// Text has been deleted, apply record coordinates
	virtual void on_delete(const position& from, const position& to) = 0;
protected:
	// id for this record to identify it after having sent it through the
	// net (the server has to say which record has been synced)
	unsigned int m_id;
	// counter to create unique ids on one host
	static unsigned int m_counter;
	// Revision of this record
	unsigned int m_revision;
	// Guy who made this record
	unsigned int m_from;
};

// insert_record is an insertion of text into the document
class insert_record : public record
{
public:
	insert_record(const position& pos, const std::string& text,
	              unsigned int revision, unsigned int from);
	insert_record(const position& pos, const std::string& text,
	              unsigned int revision, unsigned int from,
	              unsigned int id);
	~insert_record();

	virtual void apply(buffer& buf);
	virtual bool is_valid() const;

	virtual void on_insert(const position& pos, const std::string& text);
	virtual void on_delete(const position& from, const position& to);
protected:
	position pos;
	std::string text;
};

// delete_record is an erasure of text out of the buffer
class delete_record : public record
{
public:
	delete_record(const position& from, const position& to,
	              unsigned int revision, unsigned int from);
	delete_record(const position& from, const position& to,
	              unsigned int revision, unsigned int from,
	              unsigned int id);
	~delete_record();

	virtual void apply(buffer& buf);
	virtual bool is_valid() const;

	virtual void on_insert(const position& pos, const std::string& text);
	virtual void on_delete(const position& from, const position& to);
protected:
	position m_from;
	position m_to;
};

// TODO: indent_record, unindent_record

class buffer
{
public:
	buffer();
	~buffer();

	void insert(const position& pos, const std::string& text);
	void erase(const position& from, const position& to);

protected:
	// Changes to the document
	std::list<record*> m_history;
	// Document revision
	unsigned int m_revision;
	// document data
	std::vector<std::string> m_lines;
};

class client_buffer
{
public:
	typedef sigc::signal<void, const insert_record&> signal_insert_type;
	typedef sigc::signal<void, const delete_record&> signal_delete_type;
	typedef sigc::signal<void, net6::client::peer&> signal_join_type;
	typedef sigc::signal<void, net6::client::peer&> signal_part_type;
	typedef sigc::signal<void>                      signal_close_type;
	typedef sigc::signal<void, const std::string&> signal_login_failed_type;
	
	client_buffer();
	~client_bufer();

	void select();
	void select(unsigned int timeout);

	signal_insert_type& insert_event();
	signal_delete_type& delete_event();
	signal_join_type& join_event();
	signal_part_type& part_event();
	signal_close_type& close_event();
	signal_login_failed_type& login_failed_event();

protected:
	// Unsynced changes
	std::list<record*> m_unsynced;
	// Network connection
	net6::client m_connection;

	signal_insert_type m_signal_insert;
	signal_delete_type m_signal_delete;
	signal_join_type m_signal_join;
	signal_part_type m_signal_part;
	signal_close_type m_signal_close;
	signal_login_failed_type m_login_failed_event;
};

class server_buffer
{
public: 
	typedef sigc::signal<void, const insert_record&> signal_insert_type;
	typedef sigc::signal<void, const delete_record&> signal_delete_type;
	typedef sigc::signal<void, net6::server::peer&> signal_join_type;
	typedef sigc::signal<void, net6::server::peer&> signal_login_type;
	typedef sigc::signal<void, net6::server::peer&> signal_part_type;

	server_buffer();
	~server_buffer();

	void select();
	void select(unsigned int timeout);

	signal_insert_type& insert_event();
	signal_delete_type& delete_event();
	signal_join_type& join_event();
	signal_login_type& login_event();
	signal_part_type& part_event();

protected:
	// Revision counter
	unsigned int m_counter;
	// Network server object
	net6::server server;

	signal_insert_type m_signal_insert;
	signal_delete_type m_signal_delete;
	signal_join_type m_signal_join;
	signal_login_type m_signal_login;
	signal_part_type m_signal_part;
};

}
