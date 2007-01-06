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

	// TODO: move: Den Record um Zeilen/Spalten verschieben
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

protected:
	position pos;
	std::string text;
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
	// Unsynced changes (only necessary for client buffers)
	std::list<record*> m_unsynced;
	// Last changes to undo thingies
	std::list<record*> m_history;
	// Document revision
	unsigned int m_revision;
	// Counter to generate unique revision numbers (only used by server)
	static unsigned int m_counter;
};

// TODO: client_buffer / server_buffer? client_buffer has to remember unsynced
// changes, server_buffer has to have a list which all clients and their ids
// (Each client has a unique id, the from member of record is the id of
// the client which made the corresponding record)

}
