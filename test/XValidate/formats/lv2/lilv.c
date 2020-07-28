#include "lilv.h"

#include <lv2/core/lv2.h>
#include <lv2/ui/ui.h>
#include <lv2/midi/midi.h>
#include <lv2/atom/atom.h>
#include <lv2/event/event.h>
#include <lv2/atom/forge.h>
#include <lv2/presets/presets.h>
#include <lv2/state/state.h>
#include <lv2/urid/urid.h>
#ifdef LILV_DYN_MANIFEST
#    include <dynmanifest/dynmanifest.h>
#    include <dlfcn.h>
#endif

#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#    include <windows.h>
#    include <direct.h>
#    include <stdio.h>
#    define dlopen(path, flags) LoadLibrary(path)
#    define dlclose(lib)        FreeLibrary((HMODULE)lib)
#    define unlink(path)        _unlink(path)
#    define rmdir(path)         _rmdir(path)
#    ifdef _MSC_VER
#        define __func__ __FUNCTION__
#        ifndef snprintf
#            define snprintf _snprintf
#        endif
#    endif
#ifndef INFINITY
#    define INFINITY DBL_MAX + DBL_MAX
#endif
#ifndef NAN
#    define NAN INFINITY - INFINITY
#endif
static inline const char* dlerror(void) { return "Unknown error"; }
#else
#    include <dlfcn.h>
#    include <unistd.h>
#endif


#define NS_RDF (const uint8_t*)"http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define NS_XSD (const uint8_t*)"http://www.w3.org/2001/XMLSchema#"
#define USTR(str) ((const uint8_t*)(str))



/*************ENUMS*************/

// Index into a SordQuad.
typedef enum { //
    SORD_SUBJECT = 0,
    SORD_PREDICATE = 1,
    SORD_OBJECT = 2,
    SORD_GRAPH = 3
} SordQuadIndex;

//Type of a node.
typedef enum {
    SORD_URI = 1,
    SORD_BLANK = 2,
    SORD_LITERAL = 3
} SordNodeType;
//Indexing option.
typedef enum {
    SORD_SPO = 1,
    SORD_SOP = 1 << 1,
    SORD_OPS = 1 << 2,
    SORD_OSP = 1 << 3,
    SORD_PSO = 1 << 4,
    SORD_POS = 1 << 5
} SordIndexOption;

typedef enum {
    ZIX_STATUS_SUCCESS,
    ZIX_STATUS_ERROR,
    ZIX_STATUS_NO_MEM,
    ZIX_STATUS_NOT_FOUND,
    ZIX_STATUS_EXISTS,
    ZIX_STATUS_BAD_ARG,
    ZIX_STATUS_BAD_PERMS,
} ZixStatus;
// Return status code.
typedef enum {
    SERD_SUCCESS,
    SERD_FAILURE,
    SERD_ERR_UNKNOWN,
    SERD_ERR_BAD_SYNTAX,
    SERD_ERR_BAD_ARG,
    SERD_ERR_NOT_FOUND,
    SERD_ERR_ID_CLASH,
    SERD_ERR_BAD_CURIE,
    SERD_ERR_INTERNAL
} SerdStatus;

// RDF syntax type.
typedef enum {
    SERD_TURTLE = 1, //  Turtle - Terse RDF Triple Language (UTF-8).  @see <a href="http://www.w3.org/TeamSubmission/turtle/">Turtle</a>
    SERD_NTRIPLES = 2, //  NTriples - Line-based RDF triples (ASCII).   @see <a href="http://www.w3.org/TR/rdf-testcases#ntriples">NTriples</a>
    SERD_NQUADS = 3, //  NQuads - Line-based RDF quads (UTF-8). @see <a href="https://www.w3.org/TR/n-quads/">NQuads</a>
    SERD_TRIG = 4 //TriG - Terse RDF quads (UTF-8). @see <a href="https://www.w3.org/TR/trig/">Trig</a>
} SerdSyntax;

typedef enum {
    SERD_EMPTY_S = 1 << 1,
    SERD_EMPTY_O = 1 << 2,
    SERD_ANON_S_BEGIN = 1 << 3,
    SERD_ANON_O_BEGIN = 1 << 4,
    SERD_ANON_CONT = 1 << 5,
    SERD_LIST_S_BEGIN = 1 << 6,
    SERD_LIST_O_BEGIN = 1 << 7,
    SERD_LIST_CONT = 1 << 8
} SerdStatementFlag;

typedef enum {
    SERD_NOTHING = 0,
    SERD_LITERAL = 1,
    SERD_URI = 2,
    SERD_CURIE = 3,
    SERD_BLANK = 4
} SerdType;

typedef enum {
    SERD_HAS_NEWLINE = 1,
    SERD_HAS_QUOTE = 1 << 1
} SerdNodeFlag;
typedef enum {
    SERD_STYLE_ABBREVIATED = 1,
    SERD_STYLE_ASCII = 1 << 1,
    SERD_STYLE_RESOLVED = 1 << 2,
    SERD_STYLE_CURIED = 1 << 3,
    SERD_STYLE_BULK = 1 << 4
} SerdStyle;

typedef enum {
    SRATOM_OBJECT_MODE_BLANK,
    SRATOM_OBJECT_MODE_BLANK_SUBJECT
} SratomObjectMode;
static const SerdStyle style = (SerdStyle)(
    SERD_STYLE_ABBREVIATED | SERD_STYLE_RESOLVED | SERD_STYLE_CURIED);
typedef enum {
    MODE_SUBJECT,
    MODE_BODY,
    MODE_SEQUENCE
} ReadMode;

/** Triple ordering */
typedef enum {
    SPO,   ///<         Subject,   Predicate, Object
    SOP,   ///<         Subject,   Object,    Predicate
    OPS,   ///<         Object,    Predicate, Subject
    OSP,   ///<         Object,    Subject,   Predicate
    PSO,   ///<         Predicate, Subject,   Object
    POS,   ///<         Predicate, Object,    Subject
    GSPO,  ///< Graph,  Subject,   Predicate, Object
    GSOP,  ///< Graph,  Subject,   Object,    Predicate
    GOPS,  ///< Graph,  Object,    Predicate, Subject
    GOSP,  ///< Graph,  Object,    Subject,   Predicate
    GPSO,  ///< Graph,  Predicate, Subject,   Object
    GPOS   ///< Graph,  Predicate, Object,    Subject
} SordOrder;

/** Mode for searching or iteration */
typedef enum {
    ALL,           ///< Iterate over entire store
    SINGLE,        ///< Iteration over a single element (exact search)
    RANGE,         ///< Iterate over range with equal prefix
    FILTER_RANGE,  ///< Iterate over range with equal prefix, filtering
    FILTER_ALL     ///< Iterate to end of store, filtering
} SearchMode;

/*****************TYPEDEFS******************/
typedef uint32_t SerdStatementFlags;
typedef uint32_t SerdNodeFlags;

typedef struct ZixBTreeImpl ZixBTree;
typedef struct ZixBTreeNodeImpl ZixBTreeNode;
typedef struct ZixBTreeIterImpl ZixBTreeIter;
typedef struct ZixTreeImpl ZixTree;
typedef struct ZixTreeNodeImpl ZixTreeIter;
typedef struct ZixHashImpl ZixHash;
typedef struct SerdEnvImpl SerdEnv;
typedef struct SerdReaderImpl SerdReader;
typedef struct SerdWriterImpl SerdWriter;
typedef struct SordWorldImpl SordWorld;
typedef struct SordModelImpl SordModel;
typedef struct SordInserterImpl SordInserter;
typedef struct SordIterImpl SordIter;
typedef struct SordNodeImpl SordNode;
typedef const SordNode* SordQuad[4];



/****************STRUCTS******************/
typedef struct {
    const uint8_t* buf;
    size_t         n_bytes;
    size_t         n_chars;
    SerdNodeFlags  flags;
    SerdType       type;
} SerdNode;

typedef struct {
    const uint8_t* buf;
    size_t         len;
} SerdChunk;

typedef struct {
    SerdStatus     status;
    const uint8_t* filename;
    unsigned       line;
    unsigned       col;
    const char* fmt;
    va_list* args;
} SerdError;

typedef struct {
    SerdChunk scheme;
    SerdChunk authority;
    SerdChunk path_base;
    SerdChunk path;
    SerdChunk query;
    SerdChunk fragment;
} SerdURI;

typedef struct ZixHashEntry {
    struct ZixHashEntry* next;  ///< Next entry in bucket
    uint32_t             hash;  ///< Non-modulo hash value
    // Value follows here (access with zix_hash_value)
} ZixHashEntry;

typedef uint32_t(*ZixHashFunc)(const void* value);
typedef bool (*ZixEqualFunc)(const void* a, const void* b);
struct ZixHashImpl {
    ZixHashFunc     hash_func;
    ZixEqualFunc    equal_func;
    ZixHashEntry** buckets;
    const unsigned* n_buckets;
    size_t          value_size;
    unsigned        count;
};

typedef struct {
    ZixBTreeNode* node;
    unsigned      index;
} ZixBTreeIterFrame;

struct ZixBTreeIterImpl {
    unsigned          level;    ///< Current level in stack
    ZixBTreeIterFrame stack[];  ///< Position stack
};

/** Resource node metadata */
typedef struct {
    size_t refs_as_obj;  ///< References as a quad object
} SordResourceMetadata;

/** Literal node metadata */
typedef struct {
    SordNode* datatype;  ///< Optional literal data type URI
    char      lang[16];  ///< Optional language tag
} SordLiteralMetadata;

/** Node */
struct SordNodeImpl {
    SerdNode node;  ///< Serd node
    size_t   refs;  ///< Reference count (# of containing quads)
    union {
        SordResourceMetadata res;
        SordLiteralMetadata  lit;
    } meta;
};


typedef int (*ZixComparator)(const void* a, const void* b, void* user_data);



typedef void (*ZixDestroyFunc)(void* ptr);


typedef void (*ZixHashVisitFunc)(void* value, void* user_data);


typedef void (*ZixHashVisitFunc)(void* value, void* user_data);
typedef int (*SerdStreamErrorFunc)(void* stream);
typedef size_t(*SerdSource)(void* buf, size_t size, size_t nmemb, void* stream);
typedef size_t(*SerdSink)(const void* buf, size_t len, void* stream);
typedef SerdStatus(*SerdErrorSink)(void* handle, const SerdError* error);

typedef SerdStatus(*SerdBaseSink)(void* handle, const SerdNode* uri);

typedef SerdStatus(*SerdPrefixSink)(void* handle, const SerdNode* name, const SerdNode* uri);

typedef SerdStatus(*SerdStatementSink)(void* handle,
    SerdStatementFlags flags,
    const SerdNode* graph,
    const SerdNode* subject,
    const SerdNode* predicate,
    const SerdNode* object,
    const SerdNode* object_datatype,
    const SerdNode* object_lang);

typedef SerdStatus(*SerdEndSink)(void* handle, const SerdNode* node);





/*********************FUNCTIONS***************************/

uint32_t zix_digest_start(void);
uint32_t zix_digest_add(uint32_t hash, const void* buf, size_t len);

ZixHash* zix_hash_new(ZixHashFunc  hash_func,
    ZixEqualFunc equal_func,
    size_t       value_size);

void zix_hash_free(ZixHash* hash);

size_t zix_hash_size(const ZixHash* hash);

ZixStatus zix_hash_insert(ZixHash* hash,
    const void* value,
    const void** inserted);

ZixStatus zix_hash_remove(ZixHash* hash, const void* value);

const void* zix_hash_find(const ZixHash* hash, const void* value);

void zix_hash_foreach(ZixHash* hash, ZixHashVisitFunc f, void* user_data);


ZixBTree* zix_btree_new(ZixComparator  cmp, void* cmp_data, ZixDestroyFunc destroy);

void
zix_btree_free(ZixBTree* t);

size_t
zix_btree_size(const ZixBTree* t);

ZixStatus
zix_btree_insert(ZixBTree* t, void* e);

ZixStatus
zix_btree_remove(ZixBTree* t, const void* e, void** out, ZixBTreeIter** next);

ZixStatus
zix_btree_find(const ZixBTree* t, const void* e, ZixBTreeIter** ti);

ZixStatus
zix_btree_lower_bound(const ZixBTree* t, const void* e, ZixBTreeIter** ti);

void*
zix_btree_get(const ZixBTreeIter* ti);

ZixBTreeIter*
zix_btree_begin(const ZixBTree* t);

bool
zix_btree_iter_is_end(const ZixBTreeIter* i);

void
zix_btree_iter_increment(ZixBTreeIter* i);

void
zix_btree_iter_free(ZixBTreeIter* i);

ZixTree*
zix_tree_new(bool           allow_duplicates,
    ZixComparator  cmp,
    void* cmp_data,
    ZixDestroyFunc destroy);

void
zix_tree_free(ZixTree* t);

size_t
zix_tree_size(const ZixTree* t);

ZixStatus
zix_tree_insert(ZixTree* t, void* e, ZixTreeIter** ti);

ZixStatus
zix_tree_remove(ZixTree* t, ZixTreeIter* ti);

ZixStatus
zix_tree_find(const ZixTree* t, const void* e, ZixTreeIter** ti);

void*
zix_tree_get(const ZixTreeIter* ti);

ZixTreeIter*
zix_tree_begin(ZixTree* t);

ZixTreeIter*
zix_tree_end(ZixTree* t);

bool
zix_tree_iter_is_end(const ZixTreeIter* i);

ZixTreeIter*
zix_tree_rbegin(ZixTree* t);

ZixTreeIter*
zix_tree_rend(ZixTree* t);

bool
zix_tree_iter_is_rend(const ZixTreeIter* i);

ZixTreeIter*
zix_tree_iter_next(ZixTreeIter* i);

ZixTreeIter*
zix_tree_iter_prev(ZixTreeIter* i);




void
serd_free(void* ptr);


const uint8_t*
serd_strerror(SerdStatus status);

size_t
serd_strlen(const uint8_t* str, size_t* n_bytes, SerdNodeFlags* flags);

double
serd_strtod(const char* str, char** endptr);

void*
serd_base64_decode(const uint8_t* str, size_t len, size_t* size);

SerdStatus
serd_reader_read_file_handle(SerdReader* reader,
    FILE* file,
    const uint8_t* name);

static const SerdURI SERD_URI_NULL = {
    {NULL, 0}, {NULL, 0}, {NULL, 0}, {NULL, 0}, {NULL, 0}, {NULL, 0}
};

const uint8_t*
serd_uri_to_path(const uint8_t* uri);

uint8_t*
serd_file_uri_parse(const uint8_t* uri, uint8_t** hostname);

bool
serd_uri_string_has_scheme(const uint8_t* utf8);

SerdStatus
serd_uri_parse(const uint8_t* utf8, SerdURI* out);

void
serd_uri_resolve(const SerdURI* r, const SerdURI* base, SerdURI* t);

size_t
serd_uri_serialise(const SerdURI* uri, SerdSink sink, void* stream);

size_t
serd_uri_serialise_relative(const SerdURI* uri,
    const SerdURI* base,
    const SerdURI* root,
    SerdSink       sink,
    void* stream);

static const SerdNode SERD_NODE_NULL = { NULL, 0, 0, 0, SERD_NOTHING };

SerdNode
serd_node_from_string(SerdType type, const uint8_t* str);

SerdNode
serd_node_from_substring(SerdType type, const uint8_t* str, size_t len);

SerdNode
serd_node_copy(const SerdNode* node);

bool
serd_node_equals(const SerdNode* a, const SerdNode* b);

SerdNode
serd_node_new_uri_from_node(const SerdNode* uri_node,
    const SerdURI* base,
    SerdURI* out);

SerdNode
serd_node_new_uri_from_string(const uint8_t* str,
    const SerdURI* base,
    SerdURI* out);

SerdNode
serd_node_new_file_uri(const uint8_t* path,
    const uint8_t* hostname,
    SerdURI* out,
    bool           escape);

SerdNode
serd_node_new_uri(const SerdURI* uri, const SerdURI* base, SerdURI* out);

SerdNode
serd_node_new_relative_uri(const SerdURI* uri,
    const SerdURI* base,
    const SerdURI* root,
    SerdURI* out);

SerdNode
serd_node_new_decimal(double d, unsigned frac_digits);

SerdNode
serd_node_new_integer(int64_t i);

SerdNode
serd_node_new_blob(const void* buf, size_t size, bool wrap_lines);

void
serd_node_free(SerdNode* node);



SerdEnv*
serd_env_new(const SerdNode* base_uri);

void
serd_env_free(SerdEnv* env);

const SerdNode*
serd_env_get_base_uri(const SerdEnv* env,
    SerdURI* out);

SerdStatus
serd_env_set_base_uri(SerdEnv* env,
    const SerdNode* uri);

SerdStatus
serd_env_set_prefix(SerdEnv* env,
    const SerdNode* name,
    const SerdNode* uri);

SerdStatus
serd_env_set_prefix_from_strings(SerdEnv* env,
    const uint8_t* name,
    const uint8_t* uri);

bool
serd_env_qualify(const SerdEnv* env,
    const SerdNode* uri,
    SerdNode* prefix,
    SerdChunk* suffix);

SerdStatus
serd_env_expand(const SerdEnv* env,
    const SerdNode* curie,
    SerdChunk* uri_prefix,
    SerdChunk* uri_suffix);


void
serd_env_foreach(const SerdEnv* env,
    SerdPrefixSink func,
    void* handle);


SerdReader*
serd_reader_new(SerdSyntax        syntax,
    void* handle,
    void              (*free_handle)(void*),
    SerdBaseSink      base_sink,
    SerdPrefixSink    prefix_sink,
    SerdStatementSink statement_sink,
    SerdEndSink       end_sink);

void
serd_reader_set_strict(SerdReader* reader, bool strict);

void
serd_reader_set_error_sink(SerdReader* reader,
    SerdErrorSink error_sink,
    void* error_handle);


void
serd_reader_add_blank_prefix(SerdReader* reader,
    const uint8_t* prefix);

void
serd_reader_set_default_graph(SerdReader* reader,
    const SerdNode* graph);

SerdStatus
serd_reader_read_file(SerdReader* reader,
    const uint8_t* uri);

SerdStatus
serd_reader_start_stream(SerdReader* reader,
    FILE* file,
    const uint8_t* name,
    bool           bulk);

SerdStatus
serd_reader_start_source_stream(SerdReader* reader,
    SerdSource          read_func,
    SerdStreamErrorFunc error_func,
    void* stream,
    const uint8_t* name,
    size_t              page_size);


SerdStatus
serd_reader_end_stream(SerdReader* reader);


SerdStatus
serd_reader_read_source(SerdReader* reader,
    SerdSource          source,
    SerdStreamErrorFunc error,
    void* stream,
    const uint8_t* name,
    size_t              page_size);

SerdStatus
serd_reader_read_string(SerdReader* reader, const uint8_t* utf8);

void
serd_reader_free(SerdReader* reader);


SerdWriter*
serd_writer_new(SerdSyntax     syntax,
    SerdStyle      style,
    SerdEnv* env,
    const SerdURI* base_uri,
    SerdSink       ssink,
    void* stream);

void
serd_writer_free(SerdWriter* writer);

SerdEnv*
serd_writer_get_env(SerdWriter* writer);

size_t
serd_file_sink(const void* buf, size_t len, void* stream);

size_t
serd_chunk_sink(const void* buf, size_t len, void* stream);

uint8_t*
serd_chunk_sink_finish(SerdChunk* stream);

void
serd_writer_set_error_sink(SerdWriter* writer,
    SerdErrorSink error_sink,
    void* error_handle);

void
serd_writer_chop_blank_prefix(SerdWriter* writer,
    const uint8_t* prefix);

SerdStatus
serd_writer_set_base_uri(SerdWriter* writer,
    const SerdNode* uri);

SerdStatus
serd_writer_set_root_uri(SerdWriter* writer,
    const SerdNode* uri);

SerdStatus
serd_writer_set_prefix(SerdWriter* writer,
    const SerdNode* name,
    const SerdNode* uri);

SerdStatus
serd_writer_write_statement(SerdWriter* writer,
    SerdStatementFlags flags,
    const SerdNode* graph,
    const SerdNode* subject,
    const SerdNode* predicate,
    const SerdNode* object,
    const SerdNode* datatype,
    const SerdNode* lang);

SerdStatus
serd_writer_end_anon(SerdWriter* writer,
    const SerdNode* node);

SerdStatus
serd_writer_finish(SerdWriter* writer);





SordWorld*
sord_world_new(void);

void
sord_world_free(SordWorld* world);

void
sord_world_set_error_sink(SordWorld* world,
    SerdErrorSink error_sink,
    void* handle);


SordNode*
sord_new_uri(SordWorld* world, const uint8_t* uri);

SordNode*
sord_new_relative_uri(SordWorld* world,
    const uint8_t* uri,
    const uint8_t* base_uri);

SordNode*
sord_new_blank(SordWorld* world, const uint8_t* str);

SordNode*
sord_new_literal(SordWorld* world,
    SordNode* datatype,
    const uint8_t* str,
    const char* lang);

SordNode*
sord_node_copy(const SordNode* node);

void
sord_node_free(SordWorld* world, SordNode* node);

SordNodeType
sord_node_get_type(const SordNode* node);

const uint8_t*
sord_node_get_string(const SordNode* node);

const uint8_t*
sord_node_get_string_counted(const SordNode* node, size_t* bytes);

const uint8_t*
sord_node_get_string_measured(const SordNode* node,
    size_t* bytes,
    size_t* chars);

const char*
sord_node_get_language(const SordNode* node);

SordNode*
sord_node_get_datatype(const SordNode* node);

SerdNodeFlags
sord_node_get_flags(const SordNode* node);

bool
sord_node_is_inline_object(const SordNode* node);

bool
sord_node_equals(const SordNode* a,
    const SordNode* b);

const SerdNode*
sord_node_to_serd_node(const SordNode* node);

SordNode*
sord_node_from_serd_node(SordWorld* world,
    SerdEnv* env,
    const SerdNode* node,
    const SerdNode* datatype,
    const SerdNode* lang);

SordModel*
sord_new(SordWorld* world,
    unsigned  indices,
    bool      graphs);

void
sord_free(SordModel* model);

SordWorld*
sord_get_world(SordModel* model);

size_t
sord_num_nodes(const SordWorld* world);

size_t
sord_num_quads(const SordModel* model);

SordIter*
sord_begin(const SordModel* model);

SordIter*
sord_find(SordModel* model, const SordQuad pat);

SordIter*
sord_search(SordModel* model,
    const SordNode* s,
    const SordNode* p,
    const SordNode* o,
    const SordNode* g);

SordNode*
sord_get(SordModel* model,
    const SordNode* s,
    const SordNode* p,
    const SordNode* o,
    const SordNode* g);

bool
sord_ask(SordModel* model,
    const SordNode* s,
    const SordNode* p,
    const SordNode* o,
    const SordNode* g);

uint64_t
sord_count(SordModel* model,
    const SordNode* s,
    const SordNode* p,
    const SordNode* o,
    const SordNode* g);

bool
sord_contains(SordModel* model, const SordQuad pat);

bool
sord_add(SordModel* model, const SordQuad tup);

void
sord_remove(SordModel* model, const SordQuad tup);

SerdStatus
sord_erase(SordModel* model, SordIter* iter);


SordInserter*
sord_inserter_new(SordModel* model,
    SerdEnv* env);

void
sord_inserter_free(SordInserter* inserter);

SerdStatus
sord_inserter_set_base_uri(SordInserter* inserter,
    const SerdNode* uri);

SerdStatus
sord_inserter_set_prefix(SordInserter* inserter,
    const SerdNode* name,
    const SerdNode* uri);

SerdStatus
sord_inserter_write_statement(SordInserter* inserter,
    SerdStatementFlags flags,
    const SerdNode* graph,
    const SerdNode* subject,
    const SerdNode* predicate,
    const SerdNode* object,
    const SerdNode* object_datatype,
    const SerdNode* object_lang);


void
sord_iter_get(const SordIter* iter, SordQuad tup);

const SordNode*
sord_iter_get_node(const SordIter* iter, SordQuadIndex index);

const SordModel*
sord_iter_get_model(SordIter* iter);

bool
sord_iter_next(SordIter* iter);

bool
sord_iter_end(const SordIter* iter);

void
sord_iter_free(SordIter* iter);


bool
sord_quad_match(const SordQuad x, const SordQuad y);


SerdReader*
sord_new_reader(SordModel* model,
    SerdEnv* env,
    SerdSyntax syntax,
    SordNode* graph);

bool
sord_write(SordModel* model,
    SerdWriter* writer,
    SordNode* graph);

bool
sord_write_iter(SordIter* iter,
    SerdWriter* writer);



struct SratomImpl {
    LV2_URID_Map* map;
    LV2_Atom_Forge    forge;
    SerdEnv* env;
    SerdNode          base_uri;
    SerdURI           base;
    SerdStatementSink write_statement;
    SerdEndSink       end_anon;
    void* handle;
    LV2_URID          atom_Event;
    LV2_URID          atom_frameTime;
    LV2_URID          atom_beatTime;
    LV2_URID          midi_MidiEvent;
    unsigned          next_id;
    SratomObjectMode  object_mode;
    uint32_t          seq_unit;
    struct {
        SordNode* atom_childType;
        SordNode* atom_frameTime;
        SordNode* atom_beatTime;
        SordNode* rdf_first;
        SordNode* rdf_rest;
        SordNode* rdf_type;
        SordNode* rdf_value;
        SordNode* xsd_base64Binary;
    } nodes;

    bool pretty_numbers;
};

typedef struct SratomImpl Sratom;

static void
read_node(Sratom* sratom,
    LV2_Atom_Forge* forge,
    SordWorld* world,
    SordModel* model,
    const SordNode* node,
    ReadMode        mode);

Sratom*
sratom_new(LV2_URID_Map* map)
{
    Sratom* sratom = (Sratom*)calloc(1, sizeof(Sratom));
    if (sratom) {
        sratom->map = map;
        sratom->atom_Event = map->map(map->handle, LV2_ATOM__Event);
        sratom->atom_frameTime = map->map(map->handle, LV2_ATOM__frameTime);
        sratom->atom_beatTime = map->map(map->handle, LV2_ATOM__beatTime);
        sratom->midi_MidiEvent = map->map(map->handle, LV2_MIDI__MidiEvent);
        sratom->object_mode = SRATOM_OBJECT_MODE_BLANK;
        lv2_atom_forge_init(&sratom->forge, map);
    }
    return sratom;
}

void
sratom_free(Sratom* sratom)
{
    if (sratom) {
        serd_node_free(&sratom->base_uri);
        free(sratom);
    }
}

void
sratom_set_env(Sratom* sratom, SerdEnv* env)
{
    sratom->env = env;
}

void
sratom_set_sink(Sratom* sratom,
    const char* base_uri,
    SerdStatementSink sink,
    SerdEndSink       end_sink,
    void* handle)
{
    if (base_uri) {
        serd_node_free(&sratom->base_uri);
        sratom->base_uri = serd_node_new_uri_from_string(
            USTR(base_uri), NULL, NULL);
        serd_uri_parse(sratom->base_uri.buf, &sratom->base);
    }
    sratom->write_statement = sink;
    sratom->end_anon = end_sink;
    sratom->handle = handle;
}

void
sratom_set_pretty_numbers(Sratom* sratom,
    bool    pretty_numbers)
{
    sratom->pretty_numbers = pretty_numbers;
}

void
sratom_set_object_mode(Sratom* sratom,
    SratomObjectMode object_mode)
{
    sratom->object_mode = object_mode;
}

static void
gensym(SerdNode* out, char c, unsigned num)
{
    out->n_bytes = out->n_chars = snprintf(
        (char*)out->buf, 10, "%c%u", c, num);
}

static void
list_append(Sratom* sratom,
    LV2_URID_Unmap* unmap,
    unsigned* flags,
    SerdNode* s,
    SerdNode* p,
    SerdNode* node,
    uint32_t          size,
    uint32_t          type,
    const void* body)
{
    // Generate a list node
    gensym(node, 'l', sratom->next_id);
    sratom->write_statement(sratom->handle, *flags, NULL,
        s, p, node, NULL, NULL);

    // _:node rdf:first value
    *flags = SERD_LIST_CONT;
    *p = serd_node_from_string(SERD_URI, NS_RDF "first");
    sratom_write(sratom, unmap, *flags, node, p, type, size, body);

    // Set subject to node and predicate to rdf:rest for next time
    gensym(node, 'l', ++sratom->next_id);
    *s = *node;
    *p = serd_node_from_string(SERD_URI, NS_RDF "rest");
}

static void
list_end(SerdStatementSink sink,
    void* handle,
    const unsigned    flags,
    SerdNode* s,
    SerdNode* p)
{
    // _:node rdf:rest rdf:nil
    const SerdNode nil = serd_node_from_string(SERD_URI, NS_RDF "nil");
    sink(handle, flags, NULL, s, p, &nil, NULL, NULL);
}

static void
start_object(Sratom* sratom,
    uint32_t* flags,
    const SerdNode* subject,
    const SerdNode* predicate,
    const SerdNode* node,
    const char* type)
{
    if (subject && predicate) {
        sratom->write_statement(sratom->handle, *flags | SERD_ANON_O_BEGIN, NULL,
            subject, predicate, node, NULL, NULL);
        // Start abbreviating object properties
        *flags |= SERD_ANON_CONT;

        // Object is in a list, stop list abbreviating if necessary
        *flags &= ~SERD_LIST_CONT;
    }
    if (type) {
        SerdNode p = serd_node_from_string(SERD_URI, NS_RDF "type");
        SerdNode o = serd_node_from_string(SERD_URI, USTR(type));
        sratom->write_statement(sratom->handle, *flags, NULL,
            node, &p, &o, NULL, NULL);
    }
}

static bool
path_is_absolute(const char* path)
{
    return (path[0] == '/'
        || (isalpha(path[0]) && path[1] == ':'
            && (path[2] == '/' || path[2] == '\\')));
}

static SerdNode
number_type(const Sratom* sratom, const uint8_t* type)
{
    if (sratom->pretty_numbers &&
        (!strcmp((const char*)type, (const char*)NS_XSD "int") ||
            !strcmp((const char*)type, (const char*)NS_XSD "long"))) {
        return serd_node_from_string(SERD_URI, NS_XSD "integer");
    }
    else if (sratom->pretty_numbers &&
        (!strcmp((const char*)type, (const char*)NS_XSD "float") ||
            !strcmp((const char*)type, (const char*)NS_XSD "double"))) {
        return serd_node_from_string(SERD_URI, NS_XSD "decimal");
    }
    else {
        return serd_node_from_string(SERD_URI, type);
    }
}

int
sratom_write(Sratom* sratom,
    LV2_URID_Unmap* unmap,
    uint32_t        flags,
    const SerdNode* subject,
    const SerdNode* predicate,
    uint32_t        type_urid,
    uint32_t        size,
    const void* body)
{
    const char* const type = unmap->unmap(unmap->handle, type_urid);
    uint8_t           idbuf[12] = "b0000000000";
    SerdNode          id = serd_node_from_string(SERD_BLANK, idbuf);
    uint8_t           nodebuf[12] = "b0000000000";
    SerdNode          node = serd_node_from_string(SERD_BLANK, nodebuf);
    SerdNode          object = SERD_NODE_NULL;
    SerdNode          datatype = SERD_NODE_NULL;
    SerdNode          language = SERD_NODE_NULL;
    bool              new_node = false;
    if (type_urid == 0 && size == 0) {
        object = serd_node_from_string(SERD_URI, USTR(NS_RDF "nil"));
    }
    else if (type_urid == sratom->forge.String) {
        object = serd_node_from_string(SERD_LITERAL, (const uint8_t*)body);
    }
    else if (type_urid == sratom->forge.Chunk) {
        datatype = serd_node_from_string(SERD_URI, NS_XSD "base64Binary");
        object = serd_node_new_blob(body, size, true);
        new_node = true;
    }
    else if (type_urid == sratom->forge.Literal) {
        const LV2_Atom_Literal_Body* lit = (const LV2_Atom_Literal_Body*)body;
        const uint8_t* str = USTR(lit + 1);
        object = serd_node_from_string(SERD_LITERAL, str);
        if (lit->datatype) {
            datatype = serd_node_from_string(
                SERD_URI, USTR(unmap->unmap(unmap->handle, lit->datatype)));
        }
        else if (lit->lang) {
            const char* lang = unmap->unmap(unmap->handle, lit->lang);
            const char* prefix = "http://lexvo.org/id/iso639-3/";
            const size_t prefix_len = strlen(prefix);
            if (lang && !strncmp(lang, prefix, prefix_len)) {
                language = serd_node_from_string(
                    SERD_LITERAL, USTR(lang + prefix_len));
            }
            else {
                fprintf(stderr, "Unknown language URID %d\n", lit->lang);
            }
        }
    }
    else if (type_urid == sratom->forge.URID) {
        const uint32_t urid = *(const uint32_t*)body;
        const uint8_t* str = USTR(unmap->unmap(unmap->handle, urid));
        object = serd_node_from_string(SERD_URI, str);
    }
    else if (type_urid == sratom->forge.Path) {
        const uint8_t* str = USTR(body);
        if (path_is_absolute((const char*)str)) {
            new_node = true;
            object = serd_node_new_file_uri(str, NULL, NULL, true);
        }
        else {
            if (!sratom->base_uri.buf ||
                strncmp((const char*)sratom->base_uri.buf, "file://", 7)) {
                fprintf(stderr, "warning: Relative path but base is not a file URI.\n");
                fprintf(stderr, "warning: Writing ambiguous atom:Path literal.\n");
                object = serd_node_from_string(SERD_LITERAL, str);
                datatype = serd_node_from_string(SERD_URI, USTR(LV2_ATOM__Path));
            }
            else {
                new_node = true;
                SerdNode rel = serd_node_new_file_uri(str, NULL, NULL, true);
                object = serd_node_new_uri_from_node(&rel, &sratom->base, NULL);
                serd_node_free(&rel);
            }
        }
    }
    else if (type_urid == sratom->forge.URI) {
        const uint8_t* str = USTR(body);
        object = serd_node_from_string(SERD_URI, str);
    }
    else if (type_urid == sratom->forge.Int) {
        new_node = true;
        object = serd_node_new_integer(*(const int32_t*)body);
        datatype = number_type(sratom, NS_XSD "int");
    }
    else if (type_urid == sratom->forge.Long) {
        new_node = true;
        object = serd_node_new_integer(*(const int64_t*)body);
        datatype = number_type(sratom, NS_XSD "long");
    }
    else if (type_urid == sratom->forge.Float) {
        new_node = true;
        object = serd_node_new_decimal(*(const float*)body, 8);
        datatype = number_type(sratom, NS_XSD "float");
    }
    else if (type_urid == sratom->forge.Double) {
        new_node = true;
        object = serd_node_new_decimal(*(const double*)body, 16);
        datatype = number_type(sratom, NS_XSD "double");
    }
    else if (type_urid == sratom->forge.Bool) {
        const int32_t val = *(const int32_t*)body;
        datatype = serd_node_from_string(SERD_URI, NS_XSD "boolean");
        object = serd_node_from_string(SERD_LITERAL,
            USTR(val ? "true" : "false"));
    }
    else if (type_urid == sratom->midi_MidiEvent) {
        new_node = true;
        datatype = serd_node_from_string(SERD_URI, USTR(LV2_MIDI__MidiEvent));
        uint8_t* str = (uint8_t*)calloc(size * 2 + 1, 1);
        for (uint32_t i = 0; i < size; ++i) {
            snprintf((char*)str + (2 * i), size * 2 + 1, "%02X",
                (unsigned)*((const uint8_t*)body + i));
        }
        object = serd_node_from_string(SERD_LITERAL, USTR(str));
    }
    else if (type_urid == sratom->atom_Event) {
        const LV2_Atom_Event* ev = (const LV2_Atom_Event*)body;
        gensym(&id, 'e', sratom->next_id++);
        start_object(sratom, &flags, subject, predicate, &id, NULL);
        SerdNode time;
        SerdNode p;
        if (sratom->seq_unit == sratom->atom_beatTime) {
            time = serd_node_new_decimal(ev->time.beats, 16);
            p = serd_node_from_string(SERD_URI, USTR(LV2_ATOM__beatTime));
            datatype = number_type(sratom, NS_XSD "double");
        }
        else {
            time = serd_node_new_integer(ev->time.frames);
            p = serd_node_from_string(SERD_URI, USTR(LV2_ATOM__frameTime));
            datatype = number_type(sratom, NS_XSD "long");
        }
        sratom->write_statement(sratom->handle, SERD_ANON_CONT, NULL,
            &id, &p, &time, &datatype, &language);
        serd_node_free(&time);

        p = serd_node_from_string(SERD_URI, NS_RDF "value");
        sratom_write(sratom, unmap, SERD_ANON_CONT, &id, &p,
            ev->body.type, ev->body.size, LV2_ATOM_BODY(&ev->body));
        if (sratom->end_anon) {
            sratom->end_anon(sratom->handle, &id);
        }
    }
    else if (type_urid == sratom->forge.Tuple) {
        gensym(&id, 't', sratom->next_id++);
        start_object(sratom, &flags, subject, predicate, &id, type);
        SerdNode p = serd_node_from_string(SERD_URI, NS_RDF "value");
        flags |= SERD_LIST_O_BEGIN;
        LV2_ATOM_TUPLE_BODY_FOREACH(body, size, i) {
            list_append(sratom, unmap, &flags, &id, &p, &node,
                i->size, i->type, LV2_ATOM_BODY(i));
        }
        list_end(sratom->write_statement, sratom->handle, flags, &id, &p);
        if (sratom->end_anon) {
            sratom->end_anon(sratom->handle, &id);
        }
    }
    else if (type_urid == sratom->forge.Vector) {
        const LV2_Atom_Vector_Body* vec = (const LV2_Atom_Vector_Body*)body;
        gensym(&id, 'v', sratom->next_id++);
        start_object(sratom, &flags, subject, predicate, &id, type);
        SerdNode p = serd_node_from_string(SERD_URI, (const uint8_t*)LV2_ATOM__childType);
        SerdNode child_type = serd_node_from_string(
            SERD_URI, (const uint8_t*)unmap->unmap(unmap->handle, vec->child_type));
        sratom->write_statement(sratom->handle, flags, NULL, &id, &p, &child_type, NULL, NULL);
        p = serd_node_from_string(SERD_URI, NS_RDF "value");
        flags |= SERD_LIST_O_BEGIN;
        for (const char* i = (const char*)(vec + 1);
            i < (const char*)vec + size;
            i += vec->child_size) {
            list_append(sratom, unmap, &flags, &id, &p, &node,
                vec->child_size, vec->child_type, i);
        }
        list_end(sratom->write_statement, sratom->handle, flags, &id, &p);
        if (sratom->end_anon) {
            sratom->end_anon(sratom->handle, &id);
        }
    }
    else if (lv2_atom_forge_is_object_type(&sratom->forge, type_urid)) {
        const LV2_Atom_Object_Body* obj = (const LV2_Atom_Object_Body*)body;
        const char* otype = unmap->unmap(unmap->handle,
            obj->otype);

        if (lv2_atom_forge_is_blank(&sratom->forge, type_urid, obj)) {
            gensym(&id, 'b', sratom->next_id++);
            start_object(sratom, &flags, subject, predicate, &id, otype);
        }
        else {
            id = serd_node_from_string(
                SERD_URI, (const uint8_t*)unmap->unmap(unmap->handle, obj->id));
            flags = 0;
            start_object(sratom, &flags, NULL, NULL, &id, otype);
        }
        LV2_ATOM_OBJECT_BODY_FOREACH(obj, size, prop) {
            const char* const key = unmap->unmap(unmap->handle, prop->key);
            SerdNode          pred = serd_node_from_string(SERD_URI, USTR(key));
            sratom_write(sratom, unmap, flags, &id, &pred,
                prop->value.type, prop->value.size,
                LV2_ATOM_BODY(&prop->value));
        }
        if (sratom->end_anon && (flags & SERD_ANON_CONT)) {
            sratom->end_anon(sratom->handle, &id);
        }
    }
    else if (type_urid == sratom->forge.Sequence) {
        const LV2_Atom_Sequence_Body* seq = (const LV2_Atom_Sequence_Body*)body;
        gensym(&id, 'v', sratom->next_id++);
        start_object(sratom, &flags, subject, predicate, &id, type);
        SerdNode p = serd_node_from_string(SERD_URI, NS_RDF "value");
        flags |= SERD_LIST_O_BEGIN;
        LV2_ATOM_SEQUENCE_BODY_FOREACH(seq, size, ev) {
            sratom->seq_unit = seq->unit;
            list_append(sratom, unmap, &flags, &id, &p, &node,
                sizeof(LV2_Atom_Event) + ev->body.size,
                sratom->atom_Event,
                ev);
        }
        list_end(sratom->write_statement, sratom->handle, flags, &id, &p);
        if (sratom->end_anon && subject && predicate) {
            sratom->end_anon(sratom->handle, &id);
        }
    }
    else {
        gensym(&id, 'b', sratom->next_id++);
        start_object(sratom, &flags, subject, predicate, &id, type);
        SerdNode p = serd_node_from_string(SERD_URI, NS_RDF "value");
        SerdNode o = serd_node_new_blob(body, size, true);
        datatype = serd_node_from_string(SERD_URI, NS_XSD "base64Binary");
        sratom->write_statement(sratom->handle, flags, NULL, &id, &p, &o, &datatype, NULL);
        if (sratom->end_anon && subject && predicate) {
            sratom->end_anon(sratom->handle, &id);
        }
        serd_node_free(&o);
    }

    if (object.buf) {
        SerdNode def_s = serd_node_from_string(SERD_BLANK, USTR("atom"));
        SerdNode def_p = serd_node_from_string(SERD_URI, USTR(NS_RDF "value"));
        if (!subject) {
            subject = &def_s;
        }
        if (!predicate) {
            predicate = &def_p;
        }
        sratom->write_statement(sratom->handle, flags, NULL,
            subject, predicate, &object, &datatype, &language);
    }

    if (new_node) {
        serd_node_free(&object);
    }

    return 0;
}

char*
sratom_to_turtle(Sratom* sratom,
    LV2_URID_Unmap* unmap,
    const char* base_uri,
    const SerdNode* subject,
    const SerdNode* predicate,
    uint32_t        type,
    uint32_t        size,
    const void* body)
{
    SerdURI     buri = SERD_URI_NULL;
    SerdNode    base = serd_node_new_uri_from_string(USTR(base_uri), &sratom->base, &buri);
    SerdEnv* env = sratom->env ? sratom->env : serd_env_new(NULL);
    SerdChunk   str = { NULL, 0 };
    SerdWriter* writer = serd_writer_new(
        SERD_TURTLE, style, env, &buri, serd_chunk_sink, &str);

    serd_env_set_base_uri(env, &base);
    sratom_set_sink(sratom, base_uri,
        (SerdStatementSink)serd_writer_write_statement,
        (SerdEndSink)serd_writer_end_anon,
        writer);
    sratom_write(sratom, unmap, SERD_EMPTY_S,
        subject, predicate, type, size, body);
    serd_writer_finish(writer);

    serd_writer_free(writer);
    if (!sratom->env) {
        serd_env_free(env);
    }
    serd_node_free(&base);
    return (char*)serd_chunk_sink_finish(&str);
}

static void
read_list_value(Sratom* sratom,
    LV2_Atom_Forge* forge,
    SordWorld* world,
    SordModel* model,
    const SordNode* node,
    ReadMode        mode)
{
    SordNode* fst = sord_get(model, node, sratom->nodes.rdf_first, NULL, NULL);
    SordNode* rst = sord_get(model, node, sratom->nodes.rdf_rest, NULL, NULL);
    if (fst && rst) {
        read_node(sratom, forge, world, model, fst, mode);
        read_list_value(sratom, forge, world, model, rst, mode);
    }
    sord_node_free(world, rst);
    sord_node_free(world, fst);
}

static void
read_resource(Sratom* sratom,
    LV2_Atom_Forge* forge,
    SordWorld* world,
    SordModel* model,
    const SordNode* node,
    LV2_URID        otype)
{
    LV2_URID_Map* map = sratom->map;
    SordQuad      q = { node, NULL, NULL, NULL };
    SordIter* i = sord_find(model, q);
    SordQuad      match;
    for (; !sord_iter_end(i); sord_iter_next(i)) {
        sord_iter_get(i, match);
        const SordNode* p = match[SORD_PREDICATE];
        const SordNode* o = match[SORD_OBJECT];
        const char* p_uri = (const char*)sord_node_get_string(p);
        uint32_t        p_urid = map->map(map->handle, p_uri);
        if (!(sord_node_equals(p, sratom->nodes.rdf_type) &&
            sord_node_get_type(o) == SORD_URI &&
            map->map(map->handle, (const char*)sord_node_get_string(o)) == otype)) {
            lv2_atom_forge_key(forge, p_urid);
            read_node(sratom, forge, world, model, o, MODE_BODY);
        }
    }
    sord_iter_free(i);
}

static uint32_t
atom_size(Sratom* sratom, uint32_t type_urid)
{
    if (type_urid == sratom->forge.Int) {
        return sizeof(int32_t);
    }
    else if (type_urid == sratom->forge.Long) {
        return sizeof(int64_t);
    }
    else if (type_urid == sratom->forge.Float) {
        return sizeof(float);
    }
    else if (type_urid == sratom->forge.Double) {
        return sizeof(double);
    }
    else if (type_urid == sratom->forge.Bool) {
        return sizeof(int32_t);
    }
    else if (type_urid == sratom->forge.URID) {
        return sizeof(uint32_t);
    }
    return 0;
}

static void
read_literal(Sratom* sratom, LV2_Atom_Forge* forge, const SordNode* node)
{
    assert(sord_node_get_type(node) == SORD_LITERAL);

    size_t        len = 0;
    const char* str = (const char*)sord_node_get_string_counted(node, &len);
    SordNode* datatype = sord_node_get_datatype(node);
    const char* language = sord_node_get_language(node);
    if (datatype) {
        const char* type_uri = (const char*)sord_node_get_string(datatype);
        if (!strcmp(type_uri, (const char*)NS_XSD "int") ||
            !strcmp(type_uri, (const char*)NS_XSD "integer")) {
            lv2_atom_forge_int(forge, strtol(str, NULL, 10));
        }
        else if (!strcmp(type_uri, (const char*)NS_XSD "long")) {
            lv2_atom_forge_long(forge, strtol(str, NULL, 10));
        }
        else if (!strcmp(type_uri, (const char*)NS_XSD "float") ||
            !strcmp(type_uri, (const char*)NS_XSD "decimal")) {
            lv2_atom_forge_float(forge, serd_strtod(str, NULL));
        }
        else if (!strcmp(type_uri, (const char*)NS_XSD "double")) {
            lv2_atom_forge_double(forge, serd_strtod(str, NULL));
        }
        else if (!strcmp(type_uri, (const char*)NS_XSD "boolean")) {
            lv2_atom_forge_bool(forge, !strcmp(str, "true"));
        }
        else if (!strcmp(type_uri, (const char*)NS_XSD "base64Binary")) {
            size_t size = 0;
            void* body = serd_base64_decode(USTR(str), len, &size);
            lv2_atom_forge_atom(forge, size, forge->Chunk);
            lv2_atom_forge_write(forge, body, size);
            free(body);
        }
        else if (!strcmp(type_uri, LV2_ATOM__Path)) {
            lv2_atom_forge_path(forge, str, len);
        }
        else if (!strcmp(type_uri, LV2_MIDI__MidiEvent)) {
            lv2_atom_forge_atom(forge, len / 2, sratom->midi_MidiEvent);
            for (const char* s = str; s < str + len; s += 2) {
                unsigned num;
                sscanf(s, "%2X", &num);
                const uint8_t c = num;
                lv2_atom_forge_raw(forge, &c, 1);
            }
            lv2_atom_forge_pad(forge, len / 2);
        }
        else {
            lv2_atom_forge_literal(
                forge, str, len,
                sratom->map->map(sratom->map->handle, type_uri),
                0);
        }
    }
    else if (language) {
        const char* prefix = "http://lexvo.org/id/iso639-3/";
        const size_t lang_len = strlen(prefix) + strlen(language);
        char* lang_uri = (char*)calloc(lang_len + 1, 1);
        snprintf(lang_uri, lang_len + 1, "%s%s", prefix, language);
        lv2_atom_forge_literal(
            forge, str, len, 0,
            sratom->map->map(sratom->map->handle, lang_uri));
        free(lang_uri);
    }
    else {
        lv2_atom_forge_string(forge, str, len);
    }
}

static void
read_object(Sratom* sratom,
    LV2_Atom_Forge* forge,
    SordWorld* world,
    SordModel* model,
    const SordNode* node,
    ReadMode        mode)
{
    LV2_URID_Map* map = sratom->map;
    size_t        len = 0;
    const char* str = (const char*)sord_node_get_string_counted(node, &len);

    SordNode* type = sord_get(
        model, node, sratom->nodes.rdf_type, NULL, NULL);
    SordNode* value = sord_get(
        model, node, sratom->nodes.rdf_value, NULL, NULL);

    const uint8_t* type_uri = NULL;
    uint32_t       type_urid = 0;
    if (type) {
        type_uri = sord_node_get_string(type);
        type_urid = map->map(map->handle, (const char*)type_uri);
    }

    LV2_Atom_Forge_Frame frame = { 0, 0 };
    if (mode == MODE_SEQUENCE) {
        SordNode* time = sord_get(
            model, node, sratom->nodes.atom_beatTime, NULL, NULL);
        uint32_t seq_unit;
        if (time) {
            const char* time_str = (const char*)sord_node_get_string(time);
            lv2_atom_forge_beat_time(forge, serd_strtod(time_str, NULL));
            seq_unit = sratom->atom_beatTime;
        }
        else {
            time = sord_get(model, node, sratom->nodes.atom_frameTime, NULL, NULL);
            const char* time_str = time
                ? (const char*)sord_node_get_string(time)
                : "";
            lv2_atom_forge_frame_time(forge, serd_strtod(time_str, NULL));
            seq_unit = sratom->atom_frameTime;
        }
        read_node(sratom, forge, world, model, value, MODE_BODY);
        sord_node_free(world, time);
        sratom->seq_unit = seq_unit;
    }
    else if (type_urid == sratom->forge.Tuple) {
        lv2_atom_forge_tuple(forge, &frame);
        read_list_value(sratom, forge, world, model, value, MODE_BODY);
    }
    else if (type_urid == sratom->forge.Sequence) {
        const LV2_Atom_Forge_Ref ref = lv2_atom_forge_sequence_head(forge, &frame, 0);
        sratom->seq_unit = 0;
        read_list_value(sratom, forge, world, model, value, MODE_SEQUENCE);

        LV2_Atom_Sequence* seq = (LV2_Atom_Sequence*)lv2_atom_forge_deref(forge, ref);
        seq->body.unit = (sratom->seq_unit == sratom->atom_frameTime) ? 0 : sratom->seq_unit;
    }
    else if (type_urid == sratom->forge.Vector) {
        SordNode* child_type_node = sord_get(
            model, node, sratom->nodes.atom_childType, NULL, NULL);
        uint32_t child_type = map->map(
            map->handle, (const char*)sord_node_get_string(child_type_node));
        uint32_t child_size = atom_size(sratom, child_type);
        if (child_size > 0) {
            LV2_Atom_Forge_Ref ref = lv2_atom_forge_vector_head(
                forge, &frame, child_size, child_type);
            read_list_value(sratom, forge, world, model, value, MODE_BODY);
            lv2_atom_forge_pop(forge, &frame);
            frame.ref = 0;
            lv2_atom_forge_pad(forge, lv2_atom_forge_deref(forge, ref)->size);
        }
        sord_node_free(world, child_type_node);
    }
    else if (value && sord_node_equals(sord_node_get_datatype(value),
        sratom->nodes.xsd_base64Binary)) {
        size_t         vlen = 0;
        const uint8_t* vstr = sord_node_get_string_counted(value, &vlen);
        size_t         size = 0;
        void* body = serd_base64_decode(vstr, vlen, &size);
        lv2_atom_forge_atom(forge, size, type_urid);
        lv2_atom_forge_write(forge, body, size);
        free(body);
    }
    else if (sord_node_get_type(node) == SORD_URI) {
        lv2_atom_forge_object(
            forge, &frame, map->map(map->handle, str), type_urid);
        read_resource(sratom, forge, world, model, node, type_urid);
    }
    else {
        lv2_atom_forge_object(forge, &frame, 0, type_urid);
        read_resource(sratom, forge, world, model, node, type_urid);
    }

    if (frame.ref) {
        lv2_atom_forge_pop(forge, &frame);
    }
    sord_node_free(world, value);
    sord_node_free(world, type);
}

static void
read_node(Sratom* sratom,
    LV2_Atom_Forge* forge,
    SordWorld* world,
    SordModel* model,
    const SordNode* node,
    ReadMode        mode)
{
    LV2_URID_Map* map = sratom->map;
    size_t        len = 0;
    const char* str = (const char*)sord_node_get_string_counted(node, &len);
    if (sord_node_get_type(node) == SORD_LITERAL) {
        read_literal(sratom, forge, node);
    }
    else if (sord_node_get_type(node) == SORD_URI &&
        !(sratom->object_mode == SRATOM_OBJECT_MODE_BLANK_SUBJECT
            && mode == MODE_SUBJECT)) {
        if (!strcmp(str, (const char*)NS_RDF "nil")) {
            lv2_atom_forge_atom(forge, 0, 0);
        }
        else if (!strncmp(str, "file://", 7)) {
            SerdURI uri;
            serd_uri_parse((const uint8_t*)str, &uri);

            SerdNode rel = serd_node_new_relative_uri(&uri, &sratom->base, NULL, NULL);
            uint8_t* path = serd_file_uri_parse(rel.buf, NULL);
            lv2_atom_forge_path(forge, (const char*)path, strlen((const char*)path));
            serd_free(path);
            serd_node_free(&rel);
        }
        else {
            lv2_atom_forge_urid(forge, map->map(map->handle, str));
        }
    }
    else {
        read_object(sratom, forge, world, model, node, mode);
    }
}

void
sratom_read(Sratom* sratom,
    LV2_Atom_Forge* forge,
    SordWorld* world,
    SordModel* model,
    const SordNode* node)
{
    sratom->nodes.atom_childType = sord_new_uri(world, USTR(LV2_ATOM__childType));
    sratom->nodes.atom_frameTime = sord_new_uri(world, USTR(LV2_ATOM__frameTime));
    sratom->nodes.atom_beatTime = sord_new_uri(world, USTR(LV2_ATOM__beatTime));
    sratom->nodes.rdf_first = sord_new_uri(world, NS_RDF "first");
    sratom->nodes.rdf_rest = sord_new_uri(world, NS_RDF "rest");
    sratom->nodes.rdf_type = sord_new_uri(world, NS_RDF "type");
    sratom->nodes.rdf_value = sord_new_uri(world, NS_RDF "value");
    sratom->nodes.xsd_base64Binary = sord_new_uri(world, NS_XSD "base64Binary");

    sratom->next_id = 1;
    read_node(sratom, forge, world, model, node, MODE_SUBJECT);

    sord_node_free(world, sratom->nodes.xsd_base64Binary);
    sord_node_free(world, sratom->nodes.rdf_value);
    sord_node_free(world, sratom->nodes.rdf_type);
    sord_node_free(world, sratom->nodes.rdf_rest);
    sord_node_free(world, sratom->nodes.rdf_first);
    sord_node_free(world, sratom->nodes.atom_frameTime);
    sord_node_free(world, sratom->nodes.atom_beatTime);
    sord_node_free(world, sratom->nodes.atom_childType);
    memset(&sratom->nodes, 0, sizeof(sratom->nodes));
}

LV2_Atom_Forge_Ref
sratom_forge_sink(LV2_Atom_Forge_Sink_Handle handle,
    const void* buf,
    uint32_t                   size)
{
    SerdChunk* chunk = (SerdChunk*)handle;
    const LV2_Atom_Forge_Ref ref = chunk->len + 1;
    serd_chunk_sink(buf, size, chunk);
    return ref;
}

LV2_Atom*
sratom_forge_deref(LV2_Atom_Forge_Sink_Handle handle, LV2_Atom_Forge_Ref ref)
{
    SerdChunk* chunk = (SerdChunk*)handle;
    return (LV2_Atom*)(chunk->buf + ref - 1);
}

LV2_Atom*
sratom_from_turtle(Sratom* sratom,
    const char* base_uri,
    const SerdNode* subject,
    const SerdNode* predicate,
    const char* str)
{
    SerdChunk   out = { NULL, 0 };
    SerdNode    base = serd_node_new_uri_from_string(USTR(base_uri), &sratom->base, NULL);
    SordWorld* world = sord_world_new();
    SordModel* model = sord_new(world, SORD_SPO, false);
    SerdEnv* env = sratom->env ? sratom->env : serd_env_new(&base);
    SerdReader* reader = sord_new_reader(model, env, SERD_TURTLE, NULL);

    if (!serd_reader_read_string(reader, (const uint8_t*)str)) {
        SordNode* s = sord_node_from_serd_node(world, env, subject, 0, 0);
        lv2_atom_forge_set_sink(
            &sratom->forge, sratom_forge_sink, sratom_forge_deref, &out);
        if (subject && predicate) {
            SordNode* p = sord_node_from_serd_node(world, env, predicate, 0, 0);
            SordNode* o = sord_get(model, s, p, NULL, NULL);
            if (o) {
                sratom_read(sratom, &sratom->forge, world, model, o);
                sord_node_free(world, o);
            }
            else {
                fprintf(stderr, "Failed to find node\n");
            }
        }
        else {
            sratom_read(sratom, &sratom->forge, world, model, s);
        }
    }
    else {
        fprintf(stderr, "Failed to read Turtle\n");
    }

    serd_reader_free(reader);
    if (!sratom->env) {
        serd_env_free(env);
    }
    sord_free(model);
    sord_world_free(world);
    serd_node_free(&base);

    return (LV2_Atom*)out.buf;
}
/***********************SORD*****************************/

#ifdef __SSE4_2__
#    include <smmintrin.h>
#endif

uint32_t
zix_digest_start(void)
{
#ifdef __SSE4_2__
    return 1;  // CRC32 initial value
#else
    return 5381;  // DJB hash initial value
#endif
}

uint32_t
zix_digest_add(uint32_t hash, const void* const buf, const size_t len)
{
    const uint8_t* str = (const uint8_t*)buf;
#ifdef __SSE4_2__
    // SSE 4.2 CRC32
    for (size_t i = 0; i < (len / sizeof(uint32_t)); ++i) {
        hash = _mm_crc32_u32(hash, *(const uint32_t*)str);
        str += sizeof(uint32_t);
    }
    if (len & sizeof(uint16_t)) {
        hash = _mm_crc32_u16(hash, *(const uint16_t*)str);
        str += sizeof(uint16_t);
    }
    if (len & sizeof(uint8_t)) {
        hash = _mm_crc32_u8(hash, *(const uint8_t*)str);
    }
#else
    // Classic DJB hash
    for (size_t i = 0; i < len; ++i) {
        hash = (hash << 5) + hash + str[i];
    }
#endif
    return hash;
}

/**
   Primes, each slightly less than twice its predecessor, and as far away
   from powers of two as possible.
*/
static const unsigned sizes[] = {
    53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317,
    196613, 393241, 786433, 1572869, 3145739, 6291469, 12582917, 25165843,
    50331653, 100663319, 201326611, 402653189, 805306457, 1610612741, 0
};



static inline void*
zix_hash_value(ZixHashEntry* entry)
{
    return entry + 1;
}

ZixHash*
zix_hash_new(ZixHashFunc  hash_func,
    ZixEqualFunc equal_func,
    size_t       value_size)
{
    ZixHash* hash = (ZixHash*)malloc(sizeof(ZixHash));
    if (hash) {
        hash->hash_func = hash_func;
        hash->equal_func = equal_func;
        hash->n_buckets = &sizes[0];
        hash->value_size = value_size;
        hash->count = 0;
        if (!(hash->buckets = (ZixHashEntry**)calloc(*hash->n_buckets,
            sizeof(ZixHashEntry*)))) {
            free(hash);
            return NULL;
        }
    }
    return hash;
}

void
zix_hash_free(ZixHash* hash)
{
    for (unsigned b = 0; b < *hash->n_buckets; ++b) {
        ZixHashEntry* bucket = hash->buckets[b];
        for (ZixHashEntry* e = bucket; e;) {
            ZixHashEntry* next = e->next;
            free(e);
            e = next;
        }
    }

    free(hash->buckets);
    free(hash);
}

size_t
zix_hash_size(const ZixHash* hash)
{
    return hash->count;
}

static inline void
insert_entry(ZixHashEntry** bucket, ZixHashEntry* entry)
{
    entry->next = *bucket;
    *bucket = entry;
}

static inline ZixStatus
rehash(ZixHash* hash, unsigned new_n_buckets)
{
    ZixHashEntry** new_buckets = (ZixHashEntry**)calloc(
        new_n_buckets, sizeof(ZixHashEntry*));
    if (!new_buckets) {
        return ZIX_STATUS_NO_MEM;
    }

    const unsigned old_n_buckets = *hash->n_buckets;
    for (unsigned b = 0; b < old_n_buckets; ++b) {
        for (ZixHashEntry* e = hash->buckets[b]; e;) {
            ZixHashEntry* const next = e->next;
            const unsigned      h = e->hash % new_n_buckets;
            insert_entry(&new_buckets[h], e);
            e = next;
        }
    }

    free(hash->buckets);
    hash->buckets = new_buckets;

    return ZIX_STATUS_SUCCESS;
}

static inline ZixHashEntry*
find_entry(const ZixHash* hash,
    const void* key,
    const unsigned h,
    const unsigned h_nomod)
{
    for (ZixHashEntry* e = hash->buckets[h]; e; e = e->next) {
        if (e->hash == h_nomod && hash->equal_func(zix_hash_value(e), key)) {
            return e;
        }
    }
    return NULL;
}

const void*
zix_hash_find(const ZixHash* hash, const void* value)
{
    const unsigned h_nomod = hash->hash_func(value);
    const unsigned h = h_nomod % *hash->n_buckets;
    ZixHashEntry* const entry = find_entry(hash, value, h, h_nomod);
    return entry ? zix_hash_value(entry) : 0;
}

ZixStatus
zix_hash_insert(ZixHash* hash, const void* value, const void** inserted)
{
    unsigned h_nomod = hash->hash_func(value);
    unsigned h = h_nomod % *hash->n_buckets;

    ZixHashEntry* elem = find_entry(hash, value, h, h_nomod);
    if (elem) {
        assert(elem->hash == h_nomod);
        if (inserted) {
            *inserted = zix_hash_value(elem);
        }
        return ZIX_STATUS_EXISTS;
    }

    elem = (ZixHashEntry*)malloc(sizeof(ZixHashEntry) + hash->value_size);
    if (!elem) {
        return ZIX_STATUS_NO_MEM;
    }
    elem->next = NULL;
    elem->hash = h_nomod;
    memcpy(elem + 1, value, hash->value_size);

    const unsigned next_n_buckets = *(hash->n_buckets + 1);
    if (next_n_buckets != 0 && (hash->count + 1) >= next_n_buckets) {
        if (!rehash(hash, next_n_buckets)) {
            h = h_nomod % *(++hash->n_buckets);
        }
    }

    insert_entry(&hash->buckets[h], elem);
    ++hash->count;
    if (inserted) {
        *inserted = zix_hash_value(elem);
    }
    return ZIX_STATUS_SUCCESS;
}

ZixStatus
zix_hash_remove(ZixHash* hash, const void* value)
{
    const unsigned h_nomod = hash->hash_func(value);
    const unsigned h = h_nomod % *hash->n_buckets;

    ZixHashEntry** next_ptr = &hash->buckets[h];
    for (ZixHashEntry* e = hash->buckets[h]; e; e = e->next) {
        if (h_nomod == e->hash &&
            hash->equal_func(zix_hash_value(e), value)) {
            *next_ptr = e->next;
            free(e);
            return ZIX_STATUS_SUCCESS;
        }
        next_ptr = &e->next;
    }

    if (hash->n_buckets != sizes) {
        const unsigned prev_n_buckets = *(hash->n_buckets - 1);
        if (hash->count - 1 <= prev_n_buckets) {
            if (!rehash(hash, prev_n_buckets)) {
                --hash->n_buckets;
            }
        }
    }

    --hash->count;
    return ZIX_STATUS_NOT_FOUND;
}

void
zix_hash_foreach(ZixHash* hash,
    ZixHashVisitFunc f,
    void* user_data)
{
    for (unsigned b = 0; b < *hash->n_buckets; ++b) {
        ZixHashEntry* bucket = hash->buckets[b];
        for (ZixHashEntry* e = bucket; e; e = e->next) {
            f(zix_hash_value(e), user_data);
        }
    }
}

// #define ZIX_BTREE_DEBUG 1

#ifndef ZIX_BTREE_PAGE_SIZE
#    define ZIX_BTREE_PAGE_SIZE 4096
#endif

#define ZIX_BTREE_NODE_SPACE (ZIX_BTREE_PAGE_SIZE - 2 * sizeof(uint16_t))
#define ZIX_BTREE_LEAF_VALS  ((ZIX_BTREE_NODE_SPACE / sizeof(void*)) - 1)
#define ZIX_BTREE_INODE_VALS (ZIX_BTREE_LEAF_VALS / 2)

struct ZixBTreeImpl {
    ZixBTreeNode* root;
    ZixDestroyFunc destroy;
    ZixComparator  cmp;
    void* cmp_data;
    size_t         size;
    unsigned       height;  ///< Number of levels, i.e. root only has height 1
};

struct ZixBTreeNodeImpl {
    uint16_t      is_leaf;
    uint16_t      n_vals;
    // On 64-bit we rely on some padding here to get page-sized nodes
    void* vals[ZIX_BTREE_INODE_VALS];  // ZIX_BTREE_LEAF_VALS for leaves
    ZixBTreeNode* children[ZIX_BTREE_INODE_VALS + 1];  // Nonexistent for leaves
};


#ifdef ZIX_BTREE_DEBUG

void
print_node(const ZixBTreeNode* n, const char* prefix)
{
    printf("%s[", prefix);
    for (uint16_t v = 0; v < n->n_vals; ++v) {
        printf(" %lu", (uintptr_t)n->vals[v]);
    }
    printf(" ]\n");
}

void
print_tree(const ZixBTreeNode* parent, const ZixBTreeNode* node, int level)
{
    if (node) {
        if (!parent) {
            printf("TREE {\n");
        }
        for (int i = 0; i < level + 1; ++i) {
            printf("  ");
        }
        print_node(node, "");
        if (!node->is_leaf) {
            for (uint16_t i = 0; i < node->n_vals + 1; ++i) {
                print_tree(node, node->children[i], level + 1);
            }
        }
        if (!parent) {
            printf("}\n");
        }
    }
}

#endif  // ZIX_BTREE_DEBUG

ZixBTreeNode*
zix_btree_node_new(const bool leaf)
{
    assert(sizeof(ZixBTreeNode) == ZIX_BTREE_PAGE_SIZE);
    ZixBTreeNode* node = (ZixBTreeNode*)malloc(sizeof(ZixBTreeNode));
    if (node) {
        node->is_leaf = leaf;
        node->n_vals = 0;
    }
    return node;
}

ZixBTree*
zix_btree_new(const ZixComparator  cmp,
    void* const          cmp_data,
    const ZixDestroyFunc destroy)
{
    ZixBTree* t = (ZixBTree*)malloc(sizeof(ZixBTree));
    if (t) {
        t->root = zix_btree_node_new(true);
        t->destroy = destroy;
        t->cmp = cmp;
        t->cmp_data = cmp_data;
        t->size = 0;
        t->height = 1;
        if (!t->root) {
            free(t);
            return NULL;
        }
    }
    return t;
}

void
zix_btree_free_rec(ZixBTree* const t, ZixBTreeNode* const n)
{
    if (n) {
        if (t->destroy) {
            for (uint16_t i = 0; i < n->n_vals; ++i) {
                t->destroy(n->vals[i]);
            }
        }
        if (!n->is_leaf) {
            for (uint16_t i = 0; i < n->n_vals + 1; ++i) {
                zix_btree_free_rec(t, n->children[i]);
            }
        }
        free(n);
    }
}

void
zix_btree_free(ZixBTree* const t)
{
    if (t) {
        zix_btree_free_rec(t, t->root);
        free(t);
    }
}

size_t
zix_btree_size(const ZixBTree* const t)
{
    return t->size;
}

uint16_t
zix_btree_max_vals(const ZixBTreeNode* const node)
{
    return node->is_leaf ? ZIX_BTREE_LEAF_VALS : ZIX_BTREE_INODE_VALS;
}

uint16_t
zix_btree_min_vals(const ZixBTreeNode* const node)
{
    return ((zix_btree_max_vals(node) + 1) / 2) - 1;
}

/** Shift pointers in `array` of length `n` right starting at `i`. */
void
zix_btree_ainsert(void** const   array,
    const uint16_t n,
    const uint16_t i,
    void* const    e)
{
    memmove(array + i + 1, array + i, (n - i) * sizeof(e));
    array[i] = e;
}

/** Erase element `i` in `array` of length `n` and return erased element. */
void*
zix_btree_aerase(void** const array, const uint16_t n, const uint16_t i)
{
    void* const ret = array[i];
    memmove(array + i, array + i + 1, (n - i) * sizeof(ret));
    return ret;
}

/** Split lhs, the i'th child of `n`, into two nodes. */
ZixBTreeNode*
zix_btree_split_child(ZixBTreeNode* const n,
    const uint16_t      i,
    ZixBTreeNode* const lhs)
{
    assert(lhs->n_vals == zix_btree_max_vals(lhs));
    assert(n->n_vals < ZIX_BTREE_INODE_VALS);
    assert(i < n->n_vals + 1);
    assert(n->children[i] == lhs);

    const uint16_t max_n_vals = zix_btree_max_vals(lhs);
    ZixBTreeNode* rhs = zix_btree_node_new(lhs->is_leaf);
    if (!rhs) {
        return NULL;
    }

    // LHS and RHS get roughly half, less the middle value which moves up
    lhs->n_vals = max_n_vals / 2;
    rhs->n_vals = max_n_vals - lhs->n_vals - 1;

    // Copy large half of values from LHS to new RHS node
    memcpy(rhs->vals,
        lhs->vals + lhs->n_vals + 1,
        rhs->n_vals * sizeof(void*));

    // Copy large half of children from LHS to new RHS node
    if (!lhs->is_leaf) {
        memcpy(rhs->children,
            lhs->children + lhs->n_vals + 1,
            (rhs->n_vals + 1) * sizeof(ZixBTreeNode*));
    }

    // Move middle value up to parent
    zix_btree_ainsert(n->vals, n->n_vals, i, lhs->vals[lhs->n_vals]);

    // Insert new RHS node in parent at position i
    zix_btree_ainsert((void**)n->children, ++n->n_vals, i + 1, rhs);

    return rhs;
}

/** Find the first value in `n` that is not less than `e` (lower bound). */
uint16_t
zix_btree_node_find(const ZixBTree* const     t,
    const ZixBTreeNode* const n,
    const void* const         e,
    bool* const               equal)
{
    uint16_t first = 0;
    uint16_t len = n->n_vals;
    while (len > 0) {
        const uint16_t half = len >> 1;
        const uint16_t i = first + half;
        const int      cmp = t->cmp(n->vals[i], e, t->cmp_data);
        if (cmp == 0) {
            *equal = true;
            len = half;  // Keep searching for wildcard matches
        }
        else if (cmp < 0) {
            const uint16_t chop = half + 1;
            first += chop;
            len -= chop;
        }
        else {
            len = half;
        }
    }
    assert(!*equal || t->cmp(n->vals[first], e, t->cmp_data) == 0);
    return first;
}

ZixStatus
zix_btree_insert(ZixBTree* const t, void* const e)
{
    ZixBTreeNode* parent = NULL;     // Parent of n
    ZixBTreeNode* n = t->root;  // Current node
    uint16_t      i = 0;        // Index of n in parent
    while (n) {
        if (n->n_vals == zix_btree_max_vals(n)) {
            // Node is full, split to ensure there is space for a leaf split
            if (!parent) {
                // Root is full, grow tree upwards
                if (!(parent = zix_btree_node_new(false))) {
                    return ZIX_STATUS_NO_MEM;
                }
                t->root = parent;
                parent->children[0] = n;
                ++t->height;
            }

            ZixBTreeNode* const rhs = zix_btree_split_child(parent, i, n);
            if (!rhs) {
                return ZIX_STATUS_NO_MEM;
            }

            const int cmp = t->cmp(parent->vals[i], e, t->cmp_data);
            if (cmp == 0) {
                return ZIX_STATUS_EXISTS;
            }
            else if (cmp < 0) {
                // Move to new RHS
                n = rhs;
                ++i;
            }
        }

        assert(!parent || parent->children[i] == n);

        bool equal = false;
        i = zix_btree_node_find(t, n, e, &equal);
        if (equal) {
            return ZIX_STATUS_EXISTS;
        }
        else if (!n->is_leaf) {
            // Descend to child node left of value
            parent = n;
            n = n->children[i];
        }
        else {
            // Insert into internal node
            zix_btree_ainsert(n->vals, n->n_vals++, i, e);
            break;
        }
    }

    ++t->size;

    return ZIX_STATUS_SUCCESS;
}

ZixBTreeIter*
zix_btree_iter_new(const ZixBTree* const t)
{
    const size_t s = t->height * sizeof(ZixBTreeIterFrame);

    return (ZixBTreeIter*)calloc(1, sizeof(ZixBTreeIter) + s);
}

void
zix_btree_iter_set_frame(ZixBTreeIter* const ti,
    ZixBTreeNode* const n,
    const uint16_t      i)
{
    if (ti) {
        ti->stack[ti->level].node = n;
        ti->stack[ti->level].index = i;
    }
}

bool
zix_btree_node_is_minimal(ZixBTreeNode* const n)
{
    assert(n->n_vals >= zix_btree_min_vals(n));
    return n->n_vals == zix_btree_min_vals(n);
}

/** Enlarge left child by stealing a value from its right sibling. */
ZixBTreeNode*
zix_btree_rotate_left(ZixBTreeNode* const parent, const uint16_t i)
{
    ZixBTreeNode* const lhs = parent->children[i];
    ZixBTreeNode* const rhs = parent->children[i + 1];

    // Move parent value to end of LHS
    lhs->vals[lhs->n_vals++] = parent->vals[i];

    // Move first child pointer from RHS to end of LHS
    if (!lhs->is_leaf) {
        lhs->children[lhs->n_vals] = (ZixBTreeNode*)zix_btree_aerase(
            (void**)rhs->children, rhs->n_vals, 0);
    }

    // Move first value in RHS to parent
    parent->vals[i] = zix_btree_aerase(rhs->vals, --rhs->n_vals, 0);

    return lhs;
}

/** Enlarge right child by stealing a value from its left sibling. */
ZixBTreeNode*
zix_btree_rotate_right(ZixBTreeNode* const parent, const uint16_t i)
{
    ZixBTreeNode* const lhs = parent->children[i - 1];
    ZixBTreeNode* const rhs = parent->children[i];

    // Prepend parent value to RHS
    zix_btree_ainsert(rhs->vals, rhs->n_vals++, 0, parent->vals[i - 1]);

    // Move last child pointer from LHS and prepend to RHS
    if (!lhs->is_leaf) {
        zix_btree_ainsert((void**)rhs->children,
            rhs->n_vals,
            0,
            lhs->children[lhs->n_vals]);
    }

    // Move last value from LHS to parent
    parent->vals[i - 1] = lhs->vals[--lhs->n_vals];

    return rhs;
}

/** Move n[i] down, merge the left and right child, return the merged node. */
ZixBTreeNode*
zix_btree_merge(ZixBTree* const t, ZixBTreeNode* const n, const uint16_t i)
{
    ZixBTreeNode* const lhs = n->children[i];
    ZixBTreeNode* const rhs = n->children[i + 1];

    assert(zix_btree_node_is_minimal(n->children[i]));
    assert(lhs->n_vals + rhs->n_vals < zix_btree_max_vals(lhs));

    // Move parent value to end of LHS
    lhs->vals[lhs->n_vals++] = zix_btree_aerase(n->vals, n->n_vals, i);

    // Erase corresponding child pointer (to RHS) in parent
    zix_btree_aerase((void**)n->children, n->n_vals, i + 1);

    // Add everything from RHS to end of LHS
    memcpy(lhs->vals + lhs->n_vals, rhs->vals, rhs->n_vals * sizeof(void*));
    if (!lhs->is_leaf) {
        memcpy(lhs->children + lhs->n_vals,
            rhs->children,
            (rhs->n_vals + 1) * sizeof(void*));
    }
    lhs->n_vals += rhs->n_vals;

    if (--n->n_vals == 0) {
        // Root is now empty, replace it with its only child
        assert(n == t->root);
        t->root = lhs;
        free(n);
    }

    free(rhs);
    return lhs;
}

/** Remove and return the min value from the subtree rooted at `n`. */
void*
zix_btree_remove_min(ZixBTree* const t, ZixBTreeNode* n)
{
    while (!n->is_leaf) {
        if (zix_btree_node_is_minimal(n->children[0])) {
            // Leftmost child is minimal, must expand
            if (!zix_btree_node_is_minimal(n->children[1])) {
                // Child's right sibling has at least one key to steal
                n = zix_btree_rotate_left(n, 0);
            }
            else {
                // Both child and right sibling are minimal, merge
                n = zix_btree_merge(t, n, 0);
            }
        }
        else {
            n = n->children[0];
        }
    }

    return zix_btree_aerase(n->vals, --n->n_vals, 0);
}

/** Remove and return the max value from the subtree rooted at `n`. */
void*
zix_btree_remove_max(ZixBTree* const t, ZixBTreeNode* n)
{
    while (!n->is_leaf) {
        if (zix_btree_node_is_minimal(n->children[n->n_vals])) {
            // Leftmost child is minimal, must expand
            if (!zix_btree_node_is_minimal(n->children[n->n_vals - 1])) {
                // Child's left sibling has at least one key to steal
                n = zix_btree_rotate_right(n, n->n_vals);
            }
            else {
                // Both child and left sibling are minimal, merge
                n = zix_btree_merge(t, n, n->n_vals - 1);
            }
        }
        else {
            n = n->children[n->n_vals];
        }
    }

    return n->vals[--n->n_vals];
}

ZixStatus
zix_btree_remove(ZixBTree* const      t,
    const void* const    e,
    void** const         out,
    ZixBTreeIter** const next)
{
    ZixBTreeNode* n = t->root;
    ZixBTreeIter* ti = NULL;
    const bool    user_iter = next && *next;
    if (next) {
        if (!*next && !(*next = zix_btree_iter_new(t))) {
            return ZIX_STATUS_NO_MEM;
        }
        ti = *next;
        ti->level = 0;
    }

    while (true) {
        /* To remove in a single walk down, the tree is adjusted along the way
           so that the current node always has at least one more value than the
           minimum required in general. Thus, there is always room to remove
           without adjusting on the way back up. */
        assert(n == t->root || !zix_btree_node_is_minimal(n));

        bool           equal = false;
        const uint16_t i = zix_btree_node_find(t, n, e, &equal);
        zix_btree_iter_set_frame(ti, n, i);
        if (n->is_leaf) {
            if (equal) {
                // Found in leaf node
                *out = zix_btree_aerase(n->vals, --n->n_vals, i);
                if (ti && i == n->n_vals) {
                    if (i == 0) {
                        ti->stack[ti->level = 0].node = NULL;
                    }
                    else {
                        --ti->stack[ti->level].index;
                        zix_btree_iter_increment(ti);
                    }
                }
                --t->size;
                return ZIX_STATUS_SUCCESS;
            }
            else {
                // Not found in leaf node, or tree
                if (ti && !user_iter) {
                    zix_btree_iter_free(ti);
                    *next = NULL;
                }
                return ZIX_STATUS_NOT_FOUND;
            }
        }
        else if (equal) {
            // Found in internal node
            if (!zix_btree_node_is_minimal(n->children[i])) {
                // Left child can remove without merge
                *out = n->vals[i];
                n->vals[i] = zix_btree_remove_max(t, n->children[i]);
                --t->size;
                return ZIX_STATUS_SUCCESS;
            }
            else if (!zix_btree_node_is_minimal(n->children[i + 1])) {
                // Right child can remove without merge
                *out = n->vals[i];
                n->vals[i] = zix_btree_remove_min(t, n->children[i + 1]);
                --t->size;
                return ZIX_STATUS_SUCCESS;
            }
            else {
                // Both preceding and succeeding child are minimal
                n = zix_btree_merge(t, n, i);
            }
        }
        else {
            // Not found in internal node, key is in/under children[i]
            if (zix_btree_node_is_minimal(n->children[i])) {
                if (i > 0 && !zix_btree_node_is_minimal(n->children[i - 1])) {
                    // Steal a key from child's left sibling
                    n = zix_btree_rotate_right(n, i);
                }
                else if (i < n->n_vals &&
                    !zix_btree_node_is_minimal(n->children[i + 1])) {
                    // Steal a key from child's right sibling
                    n = zix_btree_rotate_left(n, i);
                }
                else {
                    // Both child's siblings are minimal, merge them
                    if (i < n->n_vals) {
                        n = zix_btree_merge(t, n, i);
                    }
                    else {
                        n = zix_btree_merge(t, n, i - 1);
                        if (ti) {
                            --ti->stack[ti->level].index;
                        }
                    }
                }
            }
            else {
                n = n->children[i];
            }
        }
        if (ti) {
            ++ti->level;
        }
    }

    assert(false);  // Not reached
    return ZIX_STATUS_ERROR;
}

ZixStatus
zix_btree_find(const ZixBTree* const t,
    const void* const     e,
    ZixBTreeIter** const  ti)
{
    ZixBTreeNode* n = t->root;
    if (!(*ti = zix_btree_iter_new(t))) {
        return ZIX_STATUS_NO_MEM;
    }

    while (n) {
        bool           equal = false;
        const uint16_t i = zix_btree_node_find(t, n, e, &equal);

        zix_btree_iter_set_frame(*ti, n, i);

        if (equal) {
            return ZIX_STATUS_SUCCESS;
        }
        else if (n->is_leaf) {
            break;
        }
        else {
            ++(*ti)->level;
            n = n->children[i];
        }
    }

    zix_btree_iter_free(*ti);
    *ti = NULL;
    return ZIX_STATUS_NOT_FOUND;
}

ZixStatus
zix_btree_lower_bound(const ZixBTree* const t,
    const void* const     e,
    ZixBTreeIter** const  ti)
{
    if (!t) {
        *ti = NULL;
        return ZIX_STATUS_BAD_ARG;
    }

    ZixBTreeNode* n = t->root;
    bool          found = false;
    unsigned      found_level = 0;
    if (!(*ti = zix_btree_iter_new(t))) {
        return ZIX_STATUS_NO_MEM;
    }

    while (n) {
        bool           equal = false;
        const uint16_t i = zix_btree_node_find(t, n, e, &equal);

        zix_btree_iter_set_frame(*ti, n, i);

        if (equal) {
            found_level = (*ti)->level;
            found = true;
        }

        if (n->is_leaf) {
            break;
        }
        else {
            ++(*ti)->level;
            n = n->children[i];
            assert(n);
        }
    }

    const ZixBTreeIterFrame* const frame = &(*ti)->stack[(*ti)->level];
    assert(frame->node);
    if (frame->index == frame->node->n_vals) {
        if (found) {
            // Found on a previous level but went too far
            (*ti)->level = found_level;
        }
        else {
            // Reached end (key is greater than everything in tree)
            (*ti)->stack[0].node = NULL;
        }
    }

    return ZIX_STATUS_SUCCESS;
}

void*
zix_btree_get(const ZixBTreeIter* const ti)
{
    const ZixBTreeIterFrame* const frame = &ti->stack[ti->level];
    assert(frame->node);
    assert(frame->index < frame->node->n_vals);
    return frame->node->vals[frame->index];
}

ZixBTreeIter*
zix_btree_begin(const ZixBTree* const t)
{
    ZixBTreeIter* const i = zix_btree_iter_new(t);
    if (!i) {
        return NULL;
    }
    else if (t->size == 0) {
        i->stack[0].node = NULL;
    }
    else {
        ZixBTreeNode* n = t->root;
        i->stack[0].node = n;
        i->stack[0].index = 0;
        while (!n->is_leaf) {
            n = n->children[0];
            ++i->level;
            i->stack[i->level].node = n;
            i->stack[i->level].index = 0;
        }
    }
    return i;
}

bool
zix_btree_iter_is_end(const ZixBTreeIter* const i)
{
    return !i || i->stack[0].node == NULL;
}

void
zix_btree_iter_increment(ZixBTreeIter* const i)
{
    ZixBTreeIterFrame* f = &i->stack[i->level];
    if (f->node->is_leaf) {
        // Leaf, move right
        assert(f->index < f->node->n_vals);
        if (++f->index == f->node->n_vals) {
            // Reached end of leaf, move up
            f = &i->stack[i->level];
            while (i->level > 0 && f->index == f->node->n_vals) {
                f = &i->stack[--i->level];
                assert(f->index <= f->node->n_vals);
            }

            if (f->index == f->node->n_vals) {
                // Reached end of tree
                assert(i->level == 0);
                f->node = NULL;
                f->index = 0;
            }
        }
    }
    else {
        // Internal node, move down to next child
        assert(f->index < f->node->n_vals);
        ZixBTreeNode* child = f->node->children[++f->index];

        f = &i->stack[++i->level];
        f->node = child;
        f->index = 0;

        // Move down and left until we hit a leaf
        while (!f->node->is_leaf) {
            child = f->node->children[0];
            f = &i->stack[++i->level];
            f->node = child;
            f->index = 0;
        }
    }
}

void
zix_btree_iter_free(ZixBTreeIter* const i)
{
    free(i);
}


#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5)
#    define SORD_UNREACHABLE() __builtin_unreachable()
#else
#    define SORD_UNREACHABLE() assert(false)
#endif


#define SORD_LOG(prefix, ...) fprintf(stderr, "[Sord::" prefix "] " __VA_ARGS__)

#ifdef SORD_DEBUG_ITER
#    define SORD_ITER_LOG(...) SORD_LOG("iter", __VA_ARGS__)
#else
#    define SORD_ITER_LOG(...)
#endif
#ifdef SORD_DEBUG_SEARCH
#    define SORD_FIND_LOG(...) SORD_LOG("search", __VA_ARGS__)
#else
#    define SORD_FIND_LOG(...)
#endif
#ifdef SORD_DEBUG_WRITE
#    define SORD_WRITE_LOG(...) SORD_LOG("write", __VA_ARGS__)
#else
#    define SORD_WRITE_LOG(...)
#endif

#define NUM_ORDERS          12
#define STATEMENT_LEN       3
#define TUP_LEN             (STATEMENT_LEN + 1)
#define DEFAULT_ORDER       SPO
#define DEFAULT_GRAPH_ORDER GSPO

#define TUP_FMT         "(%s %s %s %s)"
#define TUP_FMT_ELEM(e) ((e) ? sord_node_get_string(e) : (const uint8_t*)"*")
#define TUP_FMT_ARGS(t) \
	TUP_FMT_ELEM((t)[0]), \
	TUP_FMT_ELEM((t)[1]), \
	TUP_FMT_ELEM((t)[2]), \
	TUP_FMT_ELEM((t)[3])

#define TUP_S 0
#define TUP_P 1
#define TUP_O 2
#define TUP_G 3

#ifdef SORD_DEBUG_SEARCH
/** String name of each ordering (array indexed by SordOrder) */
static const char* const order_names[NUM_ORDERS] = {
    "spo",  "sop",  "ops",  "osp",  "pso",  "pos",
    "gspo", "gsop", "gops", "gosp", "gpso", "gpos"
};
#endif

/**
   Quads of indices for each order, from most to least significant
   (array indexed by SordOrder)
*/
static const int orderings[NUM_ORDERS][TUP_LEN] = {
    { 0, 1, 2, 3 }, { 0, 2, 1, 3 },  // SPO, SOP
    { 2, 1, 0, 3 }, { 2, 0, 1, 3 },  // OPS, OSP
    { 1, 0, 2, 3 }, { 1, 2, 0, 3 },  // PSO, POS
    { 3, 0, 1, 2 }, { 3, 0, 2, 1 },  // GSPO, GSOP
    { 3, 2, 1, 0 }, { 3, 2, 0, 1 },  // GOPS, GOSP
    { 3, 1, 0, 2 }, { 3, 1, 2, 0 }   // GPSO, GPOS
};

/** World */
struct SordWorldImpl {
    ZixHash* nodes;
    SerdErrorSink error_sink;
    void* error_handle;
};

/** Store */
struct SordModelImpl {
    SordWorld* world;

    /** Index for each possible triple ordering (may or may not exist).
     * Each index is a tree of SordQuad with the appropriate ordering.
     */
    ZixBTree* indices[NUM_ORDERS];

    size_t n_quads;
    size_t n_iters;
};


/** Iterator over some range of a store */
struct SordIterImpl {
    const SordModel* sord;               ///< Model being iterated over
    ZixBTreeIter* cur;                ///< Current DB cursor
    SordQuad         pat;                ///< Pattern (in ordering order)
    SordOrder        order;              ///< Store order (which index)
    SearchMode       mode;               ///< Iteration mode
    int              n_prefix;           ///< Prefix for RANGE and FILTER_RANGE
    bool             end;                ///< True iff reached end
    bool             skip_graphs;        ///< Iteration should ignore graphs
};

static uint32_t
sord_node_hash(const void* n)
{
    const SordNode* node = (const SordNode*)n;
    uint32_t        hash = zix_digest_start();
    hash = zix_digest_add(hash, node->node.buf, node->node.n_bytes);
    hash = zix_digest_add(hash, &node->node.type, sizeof(node->node.type));
    if (node->node.type == SERD_LITERAL) {
        hash = zix_digest_add(hash, &node->meta.lit, sizeof(node->meta.lit));
    }
    return hash;
}

static bool
sord_node_hash_equal(const void* a, const void* b)
{
    const SordNode* a_node = (const SordNode*)a;
    const SordNode* b_node = (const SordNode*)b;
    return (a_node == b_node)
        || ((a_node->node.type == b_node->node.type) &&
            (a_node->node.type != SERD_LITERAL ||
                (a_node->meta.lit.datatype == b_node->meta.lit.datatype &&
                    !strncmp(a_node->meta.lit.lang,
                        b_node->meta.lit.lang,
                        sizeof(a_node->meta.lit.lang)))) &&
            (serd_node_equals(&a_node->node, &b_node->node)));
}

static void
error(SordWorld* world, SerdStatus st, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    const SerdError e = { st, NULL, 0, 0, fmt, &args };
    if (world->error_sink) {
        world->error_sink(world->error_handle, &e);
    }
    else {
        fprintf(stderr, "error: ");
        vfprintf(stderr, fmt, args);
    }
    va_end(args);
}

SordWorld*
sord_world_new(void)
{
    SordWorld* world = (SordWorld*)malloc(sizeof(SordWorld));
    world->error_sink = NULL;
    world->error_handle = NULL;

    world->nodes = zix_hash_new(
        sord_node_hash, sord_node_hash_equal, sizeof(SordNode));

    return world;
}

static void
free_node_entry(void* value, void* user_data)
{
    SordNode* node = (SordNode*)value;
    if (node->node.type == SERD_LITERAL) {
        sord_node_free((SordWorld*)user_data, node->meta.lit.datatype);
    }
    free((uint8_t*)node->node.buf);
}

void
sord_world_free(SordWorld* world)
{
    zix_hash_foreach(world->nodes, free_node_entry, world);
    zix_hash_free(world->nodes);
    free(world);
}

void
sord_world_set_error_sink(SordWorld* world,
    SerdErrorSink error_sink,
    void* handle)
{
    world->error_sink = error_sink;
    world->error_handle = handle;
}

/** Compare nodes, considering NULL a wildcard match. */
static inline int
sord_node_compare(const SordNode* a, const SordNode* b)
{
    if (a == b || !a || !b) {
        return 0;  // Exact or wildcard match
    }
    else if (a->node.type != b->node.type) {
        return a->node.type - b->node.type;
    }

    int cmp = 0;
    switch (a->node.type) {
    case SERD_URI:
    case SERD_BLANK:
        return strcmp((const char*)a->node.buf, (const char*)b->node.buf);
    case SERD_LITERAL:
        cmp = strcmp((const char*)sord_node_get_string(a),
            (const char*)sord_node_get_string(b));
        if (cmp == 0) {
            // Note: Can't use sord_node_compare here since it does wildcards
            if (!a->meta.lit.datatype || !b->meta.lit.datatype) {
                cmp = a->meta.lit.datatype - b->meta.lit.datatype;
            }
            else {
                cmp = strcmp((const char*)a->meta.lit.datatype->node.buf,
                    (const char*)b->meta.lit.datatype->node.buf);
            }
        }
        if (cmp == 0) {
            cmp = strcmp(a->meta.lit.lang, b->meta.lit.lang);
        }
    default:
        break;
    }
    return cmp;
}

bool
sord_node_equals(const SordNode* a, const SordNode* b)
{
    return a == b;  // Nodes are interned
}

/** Return true iff IDs are equivalent, or one is a wildcard */
static inline bool
sord_id_match(const SordNode* a, const SordNode* b)
{
    return !a || !b || (a == b);
}

static inline bool
sord_quad_match_inline(const SordQuad x, const SordQuad y)
{
    return sord_id_match(x[0], y[0])
        && sord_id_match(x[1], y[1])
        && sord_id_match(x[2], y[2])
        && sord_id_match(x[3], y[3]);
}

bool
sord_quad_match(const SordQuad x, const SordQuad y)
{
    return sord_quad_match_inline(x, y);
}

/**
   Compare two quad IDs lexicographically.
   NULL IDs (equal to 0) are treated as wildcards, always less than every
   other possible ID, except itself.
*/
static int
sord_quad_compare(const void* x_ptr, const void* y_ptr, void* user_data)
{
    const int* const           ordering = (const int*)user_data;
    const SordNode* const* const x = (const SordNode* const*)x_ptr;
    const SordNode* const* const y = (const SordNode* const*)y_ptr;

    for (int i = 0; i < TUP_LEN; ++i) {
        const int idx = ordering[i];
        const int cmp = sord_node_compare(x[idx], y[idx]);
        if (cmp) {
            return cmp;
        }
    }

    return 0;
}

static inline bool
sord_iter_forward(SordIter* iter)
{
    if (!iter->skip_graphs) {
        zix_btree_iter_increment(iter->cur);
        return zix_btree_iter_is_end(iter->cur);
    }

    SordNode** key = (SordNode**)zix_btree_get(iter->cur);
    const SordQuad initial = { key[0], key[1], key[2], key[3] };
    zix_btree_iter_increment(iter->cur);
    while (!zix_btree_iter_is_end(iter->cur)) {
        key = (SordNode**)zix_btree_get(iter->cur);
        for (int i = 0; i < 3; ++i) {
            if (key[i] != initial[i]) {
                return false;
            }
        }

        zix_btree_iter_increment(iter->cur);
    }

    return true;
}

/**
   Seek forward as necessary until `iter` points at a match.
   @return true iff iterator reached end of valid range.
*/
static inline bool
sord_iter_seek_match(SordIter* iter)
{
    for (iter->end = true;
        !zix_btree_iter_is_end(iter->cur);
        sord_iter_forward(iter)) {
        const SordNode** const key = (const SordNode**)zix_btree_get(iter->cur);
        if (sord_quad_match_inline(key, iter->pat)) {
            return (iter->end = false);
        }
    }
    return true;
}

/**
   Seek forward as necessary until `iter` points at a match, or the prefix
   no longer matches iter->pat.
   @return true iff iterator reached end of valid range.
*/
static inline bool
sord_iter_seek_match_range(SordIter* iter)
{
    assert(!iter->end);

    do {
        const SordNode** key = (const SordNode**)zix_btree_get(iter->cur);

        if (sord_quad_match_inline(key, iter->pat)) {
            return false;  // Found match
        }

        for (int i = 0; i < iter->n_prefix; ++i) {
            const int idx = orderings[iter->order][i];
            if (!sord_id_match(key[idx], iter->pat[idx])) {
                iter->end = true;  // Reached end of valid range
                return true;
            }
        }
    } while (!sord_iter_forward(iter));

    return (iter->end = true);  // Reached end
}

static SordIter*
sord_iter_new(const SordModel* sord, ZixBTreeIter* cur, const SordQuad pat,
    SordOrder order, SearchMode mode, int n_prefix)
{
    SordIter* iter = (SordIter*)malloc(sizeof(SordIter));
    iter->sord = sord;
    iter->cur = cur;
    iter->order = order;
    iter->mode = mode;
    iter->n_prefix = n_prefix;
    iter->end = false;
    iter->skip_graphs = order < GSPO;
    for (int i = 0; i < TUP_LEN; ++i) {
        iter->pat[i] = pat[i];
    }

    switch (iter->mode) {
    case ALL:
    case SINGLE:
    case RANGE:
        assert(
            sord_quad_match_inline((const SordNode**)zix_btree_get(iter->cur),
                iter->pat));
        break;
    case FILTER_RANGE:
        sord_iter_seek_match_range(iter);
        break;
    case FILTER_ALL:
        sord_iter_seek_match(iter);
        break;
    }

#ifdef SORD_DEBUG_ITER
    SordQuad value;
    sord_iter_get(iter, value);
    SORD_ITER_LOG("New %p pat=" TUP_FMT " cur=" TUP_FMT " end=%d skip=%d\n",
        (void*)iter, TUP_FMT_ARGS(pat), TUP_FMT_ARGS(value),
        iter->end, iter->skip_graphs);
#endif

    ++((SordModel*)sord)->n_iters;
    return iter;
}

const SordModel*
sord_iter_get_model(SordIter* iter)
{
    return iter->sord;
}

void
sord_iter_get(const SordIter* iter, SordQuad tup)
{
    SordNode** key = (SordNode**)zix_btree_get(iter->cur);
    for (int i = 0; i < TUP_LEN; ++i) {
        tup[i] = key[i];
    }
}

const SordNode*
sord_iter_get_node(const SordIter* iter, SordQuadIndex index)
{
    return (!sord_iter_end(iter)
        ? ((SordNode**)zix_btree_get(iter->cur))[index]
        : NULL);
}

static bool
sord_iter_scan_next(SordIter* iter)
{
    if (iter->end) {
        return true;
    }

    const SordNode** key;
    if (!iter->end) {
        switch (iter->mode) {
        case ALL:
            // At the end if the cursor is (assigned above)
            break;
        case SINGLE:
            iter->end = true;
            SORD_ITER_LOG("%p reached single end\n", (void*)iter);
            break;
        case RANGE:
            SORD_ITER_LOG("%p range next\n", (void*)iter);
            // At the end if the MSNs no longer match
            key = (const SordNode**)zix_btree_get(iter->cur);
            assert(key);
            for (int i = 0; i < iter->n_prefix; ++i) {
                const int idx = orderings[iter->order][i];
                if (!sord_id_match(key[idx], iter->pat[idx])) {
                    iter->end = true;
                    SORD_ITER_LOG("%p reached non-match end\n", (void*)iter);
                    break;
                }
            }
            break;
        case FILTER_RANGE:
            // Seek forward to next match, stopping if prefix changes
            sord_iter_seek_match_range(iter);
            break;
        case FILTER_ALL:
            // Seek forward to next match
            sord_iter_seek_match(iter);
            break;
        }
    }
    else {
        SORD_ITER_LOG("%p reached index end\n", (void*)iter);
    }

    if (iter->end) {
        SORD_ITER_LOG("%p Reached end\n", (void*)iter);
        return true;
    }
    else {
#ifdef SORD_DEBUG_ITER
        SordQuad tup;
        sord_iter_get(iter, tup);
        SORD_ITER_LOG("%p Increment to " TUP_FMT "\n",
            (void*)iter, TUP_FMT_ARGS(tup));
#endif
        return false;
    }
}

bool
sord_iter_next(SordIter* iter)
{
    if (iter->end) {
        return true;
    }

    iter->end = sord_iter_forward(iter);
    return sord_iter_scan_next(iter);
}

bool
sord_iter_end(const SordIter* iter)
{
    return !iter || iter->end;
}

void
sord_iter_free(SordIter* iter)
{
    SORD_ITER_LOG("%p Free\n", (void*)iter);
    if (iter) {
        --((SordModel*)iter->sord)->n_iters;
        zix_btree_iter_free(iter->cur);
        free(iter);
    }
}

/**
   Return true iff `sord` has an index for `order`.
   If `graphs` is true, `order` will be modified to be the
   corresponding order with a G prepended (so G will be the MSN).
*/
static inline bool
sord_has_index(SordModel* model, SordOrder* order, int* n_prefix, bool graphs)
{
    if (graphs) {
        *order = (SordOrder)(*order + GSPO);
        *n_prefix += 1;
    }

    return model->indices[*order];
}

/**
   Return the best available index for a pattern.
   @param pat Pattern in standard (S P O G) order
   @param mode Set to the (best) iteration mode for iterating over results
   @param n_prefix Set to the length of the range prefix
   (for `mode` == RANGE and `mode` == FILTER_RANGE)
*/
static inline SordOrder
sord_best_index(SordModel* sord,
    const SordQuad pat,
    SearchMode* mode,
    int* n_prefix)
{
    const bool graph_search = (pat[TUP_G] != 0);

    const unsigned sig
        = (pat[0] ? 1 : 0) * 0x100
        + (pat[1] ? 1 : 0) * 0x010
        + (pat[2] ? 1 : 0) * 0x001;

    SordOrder good[2] = { (SordOrder)-1, (SordOrder)-1 };

#define PAT_CASE(sig, m, g0, g1, np) \
	case sig: \
		*mode     = m; \
		good[0]   = g0; \
		good[1]   = g1; \
		*n_prefix = np; \
		break

    // Good orderings that don't require filtering
    * mode = RANGE;
    *n_prefix = 0;
    switch (sig) {
    case 0x000:
        assert(graph_search);
        *mode = RANGE;
        *n_prefix = 1;
        return DEFAULT_GRAPH_ORDER;
    case 0x111:
        *mode = SINGLE;
        return graph_search ? DEFAULT_GRAPH_ORDER : DEFAULT_ORDER;

        PAT_CASE(0x001, RANGE, OPS, OSP, 1);
        PAT_CASE(0x010, RANGE, POS, PSO, 1);
        PAT_CASE(0x011, RANGE, OPS, POS, 2);
        PAT_CASE(0x100, RANGE, SPO, SOP, 1);
        PAT_CASE(0x101, RANGE, SOP, OSP, 2);
        PAT_CASE(0x110, RANGE, SPO, PSO, 2);
    }

    if (*mode == RANGE) {
        if (sord_has_index(sord, &good[0], n_prefix, graph_search)) {
            return good[0];
        }
        else if (sord_has_index(sord, &good[1], n_prefix, graph_search)) {
            return good[1];
        }
    }

    // Not so good orderings that require filtering, but can
    // still be constrained to a range
    switch (sig) {
        PAT_CASE(0x011, FILTER_RANGE, OSP, PSO, 1);
        PAT_CASE(0x101, FILTER_RANGE, SPO, OPS, 1);
        // SPO is always present, so 0x110 is never reached here
    default: break;
    }

    if (*mode == FILTER_RANGE) {
        if (sord_has_index(sord, &good[0], n_prefix, graph_search)) {
            return good[0];
        }
        else if (sord_has_index(sord, &good[1], n_prefix, graph_search)) {
            return good[1];
        }
    }

    if (graph_search) {
        *mode = FILTER_RANGE;
        *n_prefix = 1;
        return DEFAULT_GRAPH_ORDER;
    }
    else {
        *mode = FILTER_ALL;
        return DEFAULT_ORDER;
    }
}

SordModel*
sord_new(SordWorld* world, unsigned indices, bool graphs)
{
    SordModel* model = (SordModel*)malloc(sizeof(struct SordModelImpl));
    model->world = world;
    model->n_quads = 0;
    model->n_iters = 0;

    for (unsigned i = 0; i < (NUM_ORDERS / 2); ++i) {
        const int* const ordering = orderings[i];
        const int* const g_ordering = orderings[i + (NUM_ORDERS / 2)];

        if (indices & (1 << i)) {
            model->indices[i] = zix_btree_new(
                sord_quad_compare, (void*)ordering, NULL);
            if (graphs) {
                model->indices[i + (NUM_ORDERS / 2)] = zix_btree_new(
                    sord_quad_compare, (void*)g_ordering, NULL);
            }
            else {
                model->indices[i + (NUM_ORDERS / 2)] = NULL;
            }
        }
        else {
            model->indices[i] = NULL;
            model->indices[i + (NUM_ORDERS / 2)] = NULL;
        }
    }

    if (!model->indices[DEFAULT_ORDER]) {
        model->indices[DEFAULT_ORDER] = zix_btree_new(
            sord_quad_compare, (void*)orderings[DEFAULT_ORDER], NULL);
    }
    if (graphs && !model->indices[DEFAULT_GRAPH_ORDER]) {
        model->indices[DEFAULT_GRAPH_ORDER] = zix_btree_new(
            sord_quad_compare, (void*)orderings[DEFAULT_GRAPH_ORDER], NULL);
    }

    return model;
}

static void
sord_node_free_internal(SordWorld* world, SordNode* node)
{
    assert(node->refs == 0);

    // Cache pointer to buffer to free after node removal and destruction
    const uint8_t* const buf = node->node.buf;

    // Remove node from hash (which frees the node)
    if (zix_hash_remove(world->nodes, node)) {
        error(world, SERD_ERR_INTERNAL, "failed to remove node from hash\n");
    }

    // Free buffer
    free((uint8_t*)buf);
}

static void
sord_add_quad_ref(SordModel* model, const SordNode* node, SordQuadIndex i)
{
    if (node) {
        assert(node->refs > 0);
        ++((SordNode*)node)->refs;
        if (node->node.type != SERD_LITERAL && i == SORD_OBJECT) {
            ++((SordNode*)node)->meta.res.refs_as_obj;
        }
    }
}

static void
sord_drop_quad_ref(SordModel* model, const SordNode* node, SordQuadIndex i)
{
    if (!node) {
        return;
    }

    assert(node->refs > 0);
    if (node->node.type != SERD_LITERAL && i == SORD_OBJECT) {
        assert(node->meta.res.refs_as_obj > 0);
        --((SordNode*)node)->meta.res.refs_as_obj;
    }
    if (--((SordNode*)node)->refs == 0) {
        sord_node_free_internal(sord_get_world(model), (SordNode*)node);
    }
}

void
sord_free(SordModel* model)
{
    if (!model) {
        return;
    }

    // Free nodes
    SordQuad tup;
    SordIter* i = sord_begin(model);
    for (; !sord_iter_end(i); sord_iter_next(i)) {
        sord_iter_get(i, tup);
        for (int t = 0; t < TUP_LEN; ++t) {
            sord_drop_quad_ref(model, tup[t], (SordQuadIndex)t);
        }
    }
    sord_iter_free(i);

    // Free quads
    ZixBTreeIter* t = zix_btree_begin(model->indices[DEFAULT_ORDER]);
    for (; !zix_btree_iter_is_end(t); zix_btree_iter_increment(t)) {
        free(zix_btree_get(t));
    }
    zix_btree_iter_free(t);

    // Free indices
    for (unsigned o = 0; o < NUM_ORDERS; ++o) {
        if (model->indices[o]) {
            zix_btree_free(model->indices[o]);
        }
    }

    free(model);
}

SordWorld*
sord_get_world(SordModel* model)
{
    return model->world;
}

size_t
sord_num_quads(const SordModel* model)
{
    return model->n_quads;
}

size_t
sord_num_nodes(const SordWorld* world)
{
    return zix_hash_size(world->nodes);
}

SordIter*
sord_begin(const SordModel* model)
{
    if (sord_num_quads(model) == 0) {
        return NULL;
    }
    else {
        ZixBTreeIter* cur = zix_btree_begin(model->indices[DEFAULT_ORDER]);
        SordQuad      pat = { 0, 0, 0, 0 };
        return sord_iter_new(model, cur, pat, DEFAULT_ORDER, ALL, 0);
    }
}

SordIter*
sord_find(SordModel* model, const SordQuad pat)
{
    if (!pat[0] && !pat[1] && !pat[2] && !pat[3]) {
        return sord_begin(model);
    }

    SearchMode      mode;
    int             n_prefix;
    const SordOrder index_order = sord_best_index(model, pat, &mode, &n_prefix);

    SORD_FIND_LOG("Find " TUP_FMT "  index=%s  mode=%d  n_prefix=%d\n",
        TUP_FMT_ARGS(pat), order_names[index_order], mode, n_prefix);

    if (pat[0] && pat[1] && pat[2] && pat[3]) {
        mode = SINGLE;  // No duplicate quads (Sord is a set)
    }

    ZixBTree* const db = model->indices[index_order];
    ZixBTreeIter* cur = NULL;
    zix_btree_lower_bound(db, pat, &cur);
    if (zix_btree_iter_is_end(cur)) {
        SORD_FIND_LOG("No match found\n");
        zix_btree_iter_free(cur);
        return NULL;
    }
    const SordNode** const key = (const SordNode**)zix_btree_get(cur);
    if (!key || ((mode == RANGE || mode == SINGLE)
        && !sord_quad_match_inline(pat, key))) {
        SORD_FIND_LOG("No match found\n");
        zix_btree_iter_free(cur);
        return NULL;
    }

    return sord_iter_new(model, cur, pat, index_order, mode, n_prefix);
}

SordIter*
sord_search(SordModel* model,
    const SordNode* s,
    const SordNode* p,
    const SordNode* o,
    const SordNode* g)
{
    SordQuad pat = { s, p, o, g };
    return sord_find(model, pat);
}

SordNode*
sord_get(SordModel* model,
    const SordNode* s,
    const SordNode* p,
    const SordNode* o,
    const SordNode* g)
{
    if ((bool)s + (bool)p + (bool)o != 2) {
        return NULL;
    }

    SordIter* i = sord_search(model, s, p, o, g);
    SordNode* ret = NULL;
    if (!s) {
        ret = sord_node_copy(sord_iter_get_node(i, SORD_SUBJECT));
    }
    else if (!p) {
        ret = sord_node_copy(sord_iter_get_node(i, SORD_PREDICATE));
    }
    else if (!o) {
        ret = sord_node_copy(sord_iter_get_node(i, SORD_OBJECT));
    }

    sord_iter_free(i);
    return ret;
}

bool
sord_ask(SordModel* model,
    const SordNode* s,
    const SordNode* p,
    const SordNode* o,
    const SordNode* g)
{
    SordQuad pat = { s, p, o, g };
    return sord_contains(model, pat);
}

uint64_t
sord_count(SordModel* model,
    const SordNode* s,
    const SordNode* p,
    const SordNode* o,
    const SordNode* g)
{
    SordIter* i = sord_search(model, s, p, o, g);
    uint64_t  n = 0;
    for (; !sord_iter_end(i); sord_iter_next(i)) {
        ++n;
    }
    sord_iter_free(i);
    return n;
}

bool
sord_contains(SordModel* model, const SordQuad pat)
{
    SordIter* iter = sord_find(model, pat);
    bool      ret = (iter != NULL);
    sord_iter_free(iter);
    return ret;
}

static uint8_t*
sord_strndup(const uint8_t* str, size_t len)
{
    uint8_t* dup = (uint8_t*)malloc(len + 1);
    memcpy(dup, str, len + 1);
    return dup;
}

SordNodeType
sord_node_get_type(const SordNode* node)
{
    switch (node->node.type) {
    case SERD_URI:
        return SORD_URI;
    case SERD_BLANK:
        return SORD_BLANK;
    default:
        return SORD_LITERAL;
    }
    SORD_UNREACHABLE();
}

const uint8_t*
sord_node_get_string(const SordNode* node)
{
    return node->node.buf;
}

const uint8_t*
sord_node_get_string_counted(const SordNode* node, size_t* bytes)
{
    *bytes = node->node.n_bytes;
    return node->node.buf;
}

const uint8_t*
sord_node_get_string_measured(const SordNode* node,
    size_t* bytes,
    size_t* chars)
{
    *bytes = node->node.n_bytes;
    *chars = node->node.n_chars;
    return node->node.buf;
}

const char*
sord_node_get_language(const SordNode* node)
{
    if (node->node.type != SERD_LITERAL || !node->meta.lit.lang[0]) {
        return NULL;
    }
    return node->meta.lit.lang;
}

SordNode*
sord_node_get_datatype(const SordNode* node)
{
    return (node->node.type == SERD_LITERAL) ? node->meta.lit.datatype : NULL;
}

SerdNodeFlags
sord_node_get_flags(const SordNode* node)
{
    return node->node.flags;
}

bool
sord_node_is_inline_object(const SordNode* node)
{
    return (node->node.type == SERD_BLANK) && (node->meta.res.refs_as_obj == 1);
}

static SordNode*
sord_insert_node(SordWorld* world, const SordNode* key, bool copy)
{
    SordNode* node = NULL;
    ZixStatus st = zix_hash_insert(world->nodes, key, (const void**)&node);
    switch (st) {
    case ZIX_STATUS_EXISTS:
        ++node->refs;
        break;
    case ZIX_STATUS_SUCCESS:
        assert(node->refs == 1);
        if (copy) {
            node->node.buf = sord_strndup(node->node.buf, node->node.n_bytes);
        }
        if (node->node.type == SERD_LITERAL) {
            node->meta.lit.datatype = sord_node_copy(node->meta.lit.datatype);
        }
        return node;
    default:
        error(world, SERD_ERR_INTERNAL,
            "error inserting node `%s'\n", key->node.buf);
    }

    if (!copy) {
        // Free the buffer we would have copied if a new node was created
        free((uint8_t*)key->node.buf);
    }

    return node;
}

static SordNode*
sord_new_uri_counted(SordWorld* world, const uint8_t* str,
    size_t n_bytes, size_t n_chars, bool copy)
{
    if (!serd_uri_string_has_scheme(str)) {
        error(world, SERD_ERR_BAD_ARG,
            "attempt to map invalid URI `%s'\n", str);
        return NULL;  // Can't intern relative URIs
    }

    const SordNode key = {
        { str, n_bytes, n_chars, 0, SERD_URI }, 1, { { 0 } }
    };

    return sord_insert_node(world, &key, copy);
}

SordNode*
sord_new_uri(SordWorld* world, const uint8_t* uri)
{
    const SerdNode node = serd_node_from_string(SERD_URI, uri);
    return sord_new_uri_counted(world, uri, node.n_bytes, node.n_chars, true);
}

SordNode*
sord_new_relative_uri(SordWorld* world,
    const uint8_t* uri,
    const uint8_t* base_uri)
{
    if (serd_uri_string_has_scheme(uri)) {
        return sord_new_uri(world, uri);
    }
    SerdURI  buri = SERD_URI_NULL;
    SerdNode base = serd_node_new_uri_from_string(base_uri, NULL, &buri);
    SerdNode node = serd_node_new_uri_from_string(uri, &buri, NULL);

    SordNode* ret = sord_new_uri_counted(
        world, node.buf, node.n_bytes, node.n_chars, false);

    serd_node_free(&base);
    return ret;
}

static SordNode*
sord_new_blank_counted(SordWorld* world, const uint8_t* str,
    size_t n_bytes, size_t n_chars)
{
    const SordNode key = {
        { str, n_bytes, n_chars, 0, SERD_BLANK }, 1, { { 0 } }
    };

    return sord_insert_node(world, &key, true);
}

SordNode*
sord_new_blank(SordWorld* world, const uint8_t* str)
{
    const SerdNode node = serd_node_from_string(SERD_URI, str);
    return sord_new_blank_counted(world, str, node.n_bytes, node.n_chars);
}

static SordNode*
sord_new_literal_counted(SordWorld* world,
    SordNode* datatype,
    const uint8_t* str,
    size_t         n_bytes,
    size_t         n_chars,
    SerdNodeFlags  flags,
    const char* lang)
{
    SordNode key = {
        { str, n_bytes, n_chars, flags, SERD_LITERAL }, 1, { { 0 } }
    };
    key.meta.lit.datatype = sord_node_copy(datatype);
    memset(key.meta.lit.lang, 0, sizeof(key.meta.lit.lang));
    if (lang) {
        strncpy(key.meta.lit.lang, lang, sizeof(key.meta.lit.lang));
    }

    return sord_insert_node(world, &key, true);
}

SordNode*
sord_new_literal(SordWorld* world, SordNode* datatype,
    const uint8_t* str, const char* lang)
{
    SerdNodeFlags flags = 0;
    size_t        n_bytes = 0;
    size_t        n_chars = serd_strlen(str, &n_bytes, &flags);
    return sord_new_literal_counted(world, datatype,
        str, n_bytes, n_chars, flags,
        lang);
}

SordNode*
sord_node_from_serd_node(SordWorld* world,
    SerdEnv* env,
    const SerdNode* node,
    const SerdNode* datatype,
    const SerdNode* lang)
{
    if (!node) {
        return NULL;
    }

    SordNode* datatype_node = NULL;
    SordNode* ret = NULL;
    switch (node->type) {
    case SERD_NOTHING:
        return NULL;
    case SERD_LITERAL:
        datatype_node = sord_node_from_serd_node(
            world, env, datatype, NULL, NULL),
            ret = sord_new_literal_counted(
                world,
                datatype_node,
                node->buf,
                node->n_bytes,
                node->n_chars,
                node->flags,
                lang ? (const char*)lang->buf : NULL);
        sord_node_free(world, datatype_node);
        return ret;
    case SERD_URI:
        if (serd_uri_string_has_scheme(node->buf)) {
            return sord_new_uri_counted(
                world, node->buf, node->n_bytes, node->n_chars, true);
        }
        else {
            SerdURI base_uri;
            serd_env_get_base_uri(env, &base_uri);
            SerdURI  abs_uri;
            SerdNode abs_uri_node = serd_node_new_uri_from_node(
                node, &base_uri, &abs_uri);
            ret = sord_new_uri_counted(world,
                abs_uri_node.buf,
                abs_uri_node.n_bytes,
                abs_uri_node.n_chars,
                true);
            serd_node_free(&abs_uri_node);
            return ret;
        }
    case SERD_CURIE: {
        SerdChunk uri_prefix;
        SerdChunk uri_suffix;
        if (serd_env_expand(env, node, &uri_prefix, &uri_suffix)) {
            error(world, SERD_ERR_BAD_CURIE,
                "failed to expand CURIE `%s'\n", node->buf);
            return NULL;
        }
        const size_t uri_len = uri_prefix.len + uri_suffix.len;
        uint8_t* buf = (uint8_t*)malloc(uri_len + 1);
        memcpy(buf, uri_prefix.buf, uri_prefix.len);
        memcpy(buf + uri_prefix.len, uri_suffix.buf, uri_suffix.len);
        buf[uri_len] = '\0';
        ret = sord_new_uri_counted(
            world, buf, uri_len, serd_strlen(buf, NULL, NULL), false);
        return ret;
    }
    case SERD_BLANK:
        return sord_new_blank_counted(
            world, node->buf, node->n_bytes, node->n_chars);
    }
    return NULL;
}

const SerdNode*
sord_node_to_serd_node(const SordNode* node)
{
    return node ? &node->node : &SERD_NODE_NULL;
}

void
sord_node_free(SordWorld* world, SordNode* node)
{
    if (!node) {
        return;
    }
    else if (node->refs == 0) {
        error(world, SERD_ERR_BAD_ARG, "attempt to free garbage node\n");
    }
    else if (--node->refs == 0) {
        sord_node_free_internal(world, node);
    }
}

SordNode*
sord_node_copy(const SordNode* node)
{
    SordNode* copy = (SordNode*)node;
    if (copy) {
        ++copy->refs;
    }
    return copy;
}

static inline bool
sord_add_to_index(SordModel* model, const SordNode** tup, SordOrder order)
{
    return !zix_btree_insert(model->indices[order], tup);
}

bool
sord_add(SordModel* model, const SordQuad tup)
{
    SORD_WRITE_LOG("Add " TUP_FMT "\n", TUP_FMT_ARGS(tup));
    if (!tup[0] || !tup[1] || !tup[2]) {
        error(model->world, SERD_ERR_BAD_ARG,
            "attempt to add quad with NULL field\n");
        return false;
    }
    else if (model->n_iters > 0) {
        error(model->world, SERD_ERR_BAD_ARG, "added tuple during iteration\n");
    }

    const SordNode** quad = (const SordNode**)malloc(sizeof(SordQuad));
    memcpy(quad, tup, sizeof(SordQuad));

    for (unsigned i = 0; i < NUM_ORDERS; ++i) {
        if (model->indices[i] && (i < GSPO || tup[3])) {
            if (!sord_add_to_index(model, quad, (SordOrder)i)) {
                assert(i == 0);  // Assuming index coherency
                free(quad);
                return false;  // Quad already stored, do nothing
            }
        }
    }

    for (int i = 0; i < TUP_LEN; ++i) {
        sord_add_quad_ref(model, tup[i], (SordQuadIndex)i);
    }

    ++model->n_quads;
    return true;
}

void
sord_remove(SordModel* model, const SordQuad tup)
{
    SORD_WRITE_LOG("Remove " TUP_FMT "\n", TUP_FMT_ARGS(tup));
    if (model->n_iters > 0) {
        error(model->world, SERD_ERR_BAD_ARG, "remove with iterator\n");
    }

    SordNode* quad = NULL;
    for (unsigned i = 0; i < NUM_ORDERS; ++i) {
        if (model->indices[i] && (i < GSPO || tup[3])) {
            if (zix_btree_remove(model->indices[i], tup, (void**)&quad, NULL)) {
                assert(i == 0);  // Assuming index coherency
                return;  // Quad not found, do nothing
            }
        }
    }

    free(quad);

    for (int i = 0; i < TUP_LEN; ++i) {
        sord_drop_quad_ref(model, tup[i], (SordQuadIndex)i);
    }

    --model->n_quads;
}

SerdStatus
sord_erase(SordModel* model, SordIter* iter)
{
    if (model->n_iters > 1) {
        error(model->world, SERD_ERR_BAD_ARG, "erased with many iterators\n");
        return SERD_ERR_BAD_ARG;
    }

    SordQuad tup;
    sord_iter_get(iter, tup);

    SORD_WRITE_LOG("Remove " TUP_FMT "\n", TUP_FMT_ARGS(tup));

    SordNode* quad = NULL;
    for (unsigned i = 0; i < NUM_ORDERS; ++i) {
        if (model->indices[i] && (i < GSPO || tup[3])) {
            if (zix_btree_remove(model->indices[i], tup, (void**)&quad,
                i == iter->order ? &iter->cur : NULL)) {
                return (i == 0) ? SERD_ERR_NOT_FOUND : SERD_ERR_INTERNAL;
            }
        }
    }
    iter->end = zix_btree_iter_is_end(iter->cur);
    sord_iter_scan_next(iter);

    free(quad);

    for (int i = 0; i < TUP_LEN; ++i) {
        sord_drop_quad_ref(model, tup[i], (SordQuadIndex)i);
    }

    --model->n_quads;
    return SERD_SUCCESS;
}
struct SordInserterImpl {
    SordModel* model;
    SerdEnv* env;
};

SordInserter*
sord_inserter_new(SordModel* model,
    SerdEnv* env)
{
    SordInserter* inserter = (SordInserter*)malloc(sizeof(SordInserter));
    inserter->model = model;
    inserter->env = env;
    return inserter;
}

void
sord_inserter_free(SordInserter* inserter)
{
    free(inserter);
}

SerdStatus
sord_inserter_set_base_uri(SordInserter* inserter,
    const SerdNode* uri)
{
    return serd_env_set_base_uri(inserter->env, uri);
}

SerdStatus
sord_inserter_set_prefix(SordInserter* inserter,
    const SerdNode* name,
    const SerdNode* uri)
{
    return serd_env_set_prefix(inserter->env, name, uri);
}

SerdStatus
sord_inserter_write_statement(SordInserter* inserter,
    SerdStatementFlags flags,
    const SerdNode* graph,
    const SerdNode* subject,
    const SerdNode* predicate,
    const SerdNode* object,
    const SerdNode* object_datatype,
    const SerdNode* object_lang)
{
    SordWorld* world = sord_get_world(inserter->model);
    SerdEnv* env = inserter->env;

    SordNode* g = sord_node_from_serd_node(world, env, graph, NULL, NULL);
    SordNode* s = sord_node_from_serd_node(world, env, subject, NULL, NULL);
    SordNode* p = sord_node_from_serd_node(world, env, predicate, NULL, NULL);
    SordNode* o = sord_node_from_serd_node(world, env, object,
        object_datatype, object_lang);

    if (!s || !p || !o) {
        return SERD_ERR_BAD_ARG;
    }

    const SordQuad tup = { s, p, o, g };
    sord_add(inserter->model, tup);

    sord_node_free(world, o);
    sord_node_free(world, p);
    sord_node_free(world, s);
    sord_node_free(world, g);

    return SERD_SUCCESS;
}


SerdReader*
sord_new_reader(SordModel* model,
    SerdEnv* env,
    SerdSyntax syntax,
    SordNode* graph)
{
    SordInserter* inserter = sord_inserter_new(model, env);

    SerdReader* reader = serd_reader_new(
        syntax, inserter, (void (*)(void* ptr))sord_inserter_free,
        (SerdBaseSink)sord_inserter_set_base_uri,
        (SerdPrefixSink)sord_inserter_set_prefix,
        (SerdStatementSink)sord_inserter_write_statement,
        NULL);

    if (graph) {
        serd_reader_set_default_graph(reader, sord_node_to_serd_node(graph));
    }

    return reader;
}

static SerdStatus
write_statement(SordModel* sord,
    SerdWriter* writer,
    SordQuad           tup,
    SerdStatementFlags flags)
{
    const SordNode* s = tup[SORD_SUBJECT];
    const SordNode* p = tup[SORD_PREDICATE];
    const SordNode* o = tup[SORD_OBJECT];
    const SordNode* d = sord_node_get_datatype(o);
    const SerdNode* ss = sord_node_to_serd_node(s);
    const SerdNode* sp = sord_node_to_serd_node(p);
    const SerdNode* so = sord_node_to_serd_node(o);
    const SerdNode* sd = sord_node_to_serd_node(d);

    const char* lang_str = sord_node_get_language(o);
    size_t      lang_len = lang_str ? strlen(lang_str) : 0;
    SerdNode    language = SERD_NODE_NULL;
    if (lang_str) {
        language.type = SERD_LITERAL;
        language.n_bytes = lang_len;
        language.n_chars = lang_len;
        language.buf = (const uint8_t*)lang_str;
    };

    // TODO: Subject abbreviation

    if (sord_node_is_inline_object(s) && !(flags & SERD_ANON_CONT)) {
        return SERD_SUCCESS;
    }

    SerdStatus st = SERD_SUCCESS;
    if (sord_node_is_inline_object(o)) {
        SordQuad  sub_pat = { o, 0, 0, 0 };
        SordIter* sub_iter = sord_find(sord, sub_pat);

        SerdStatementFlags start_flags = flags
            | ((sub_iter) ? SERD_ANON_O_BEGIN : SERD_EMPTY_O);

        st = serd_writer_write_statement(
            writer, start_flags, NULL, ss, sp, so, sd, &language);

        if (!st && sub_iter) {
            flags |= SERD_ANON_CONT;
            for (; !st && !sord_iter_end(sub_iter); sord_iter_next(sub_iter)) {
                SordQuad sub_tup;
                sord_iter_get(sub_iter, sub_tup);
                st = write_statement(sord, writer, sub_tup, flags);
            }
            sord_iter_free(sub_iter);
            serd_writer_end_anon(writer, so);
        }
    }
    else {
        st = serd_writer_write_statement(
            writer, flags, NULL, ss, sp, so, sd, &language);
    }

    return st;
}

bool
sord_write(SordModel* model,
    SerdWriter* writer,
    SordNode* graph)
{
    SordQuad  pat = { 0, 0, 0, graph };
    SordIter* iter = sord_find(model, pat);
    return sord_write_iter(iter, writer);
}

bool
sord_write_iter(SordIter* iter,
    SerdWriter* writer)
{
    if (!iter) {
        return false;
    }

    SordModel* model = (SordModel*)sord_iter_get_model(iter);
    SerdStatus st = SERD_SUCCESS;
    for (; !st && !sord_iter_end(iter); sord_iter_next(iter)) {
        SordQuad tup;
        sord_iter_get(iter, tup);
        st = write_statement(model, writer, tup, 0);
    }
    sord_iter_free(iter);

    return !st;
}

/***********************SRATOM*****************************/

    /**
       @defgroup sratom Sratom

       A library for serialising LV2 Atoms.

       @{
    */

    /**
       Atom serialiser.
    */
typedef struct SratomImpl Sratom;

/**
   Mode for reading resources to LV2 Objects.

   This affects how resources (which are either blank nodes or have URIs) are
   read by sratom_read(), since they may be read as simple references (a URI or
   blank node ID) or a complete description (an atom "Object").

   Currently, blank nodes are always read as Objects, but support for reading
   blank node IDs may be added in the future.
*/

/**
   Create a new Atom serialiser.
*/

Sratom*
sratom_new(LV2_URID_Map* map);

/**
   Free an Atom serialisation.
*/

void
sratom_free(Sratom* sratom);

/**
   Set the environment for reading or writing Turtle.

   This can be used to set namespace prefixes and a base URI for
   sratom_to_turtle() and sratom_from_turtle().
*/

void
sratom_set_env(Sratom* sratom,
    SerdEnv* env);

/**
   Set the sink(s) where sratom will write its output.

   This must be called before calling sratom_write().
*/

void
sratom_set_sink(Sratom* sratom,
    const char* base_uri,
    SerdStatementSink sink,
    SerdEndSink       end_sink,
    void* handle);

/**
   Write pretty numeric literals.

   If `pretty_numbers` is true, numbers will be written as pretty Turtle
   literals, rather than string literals with precise types.  The cost of this
   is that the types might get fudged on a round-trip to RDF and back.
*/

void
sratom_set_pretty_numbers(Sratom* sratom,
    bool    pretty_numbers);

/**
   Configure how resources will be read to form LV2 Objects.
*/

void
sratom_set_object_mode(Sratom* sratom,
    SratomObjectMode object_mode);

/**
   Write an Atom to RDF.
   The serialised atom is written to the sink set by sratom_set_sink().
   @return 0 on success, or a non-zero error code otherwise.
*/

int
sratom_write(Sratom* sratom,
    LV2_URID_Unmap* unmap,
    uint32_t        flags,
    const SerdNode* subject,
    const SerdNode* predicate,
    uint32_t        type_urid,
    uint32_t        size,
    const void* body);

/**
   Read an Atom from RDF.
   The resulting atom will be written to `forge`.
*/

void
sratom_read(Sratom* sratom,
    LV2_Atom_Forge* forge,
    SordWorld* world,
    SordModel* model,
    const SordNode* node);

/**
   Serialise an Atom to a Turtle string.
   The returned string must be free()'d by the caller.
*/

char*
sratom_to_turtle(Sratom* sratom,
    LV2_URID_Unmap* unmap,
    const char* base_uri,
    const SerdNode* subject,
    const SerdNode* predicate,
    uint32_t        type,
    uint32_t        size,
    const void* body);

/**
   Read an Atom from a Turtle string.
   The returned atom must be free()'d by the caller.
*/

LV2_Atom*
sratom_from_turtle(Sratom* sratom,
    const char* base_uri,
    const SerdNode* subject,
    const SerdNode* predicate,
    const char* str);

/**
   A convenient resizing sink for LV2_Atom_Forge.
   The handle must point to an initialized SerdChunk.
*/

LV2_Atom_Forge_Ref
sratom_forge_sink(LV2_Atom_Forge_Sink_Handle handle,
    const void* buf,
    uint32_t                   size);

/**
   The corresponding deref function for sratom_forge_sink.
*/

LV2_Atom*
sratom_forge_deref(LV2_Atom_Forge_Sink_Handle handle,
    LV2_Atom_Forge_Ref         ref);

/**
   @}
*/



/***********************LILV********************/



/*
 *
 * Types
 *
 */

typedef struct LilvSpecImpl LilvSpec;

typedef void LilvCollection;

struct LilvPortImpl {
    LilvNode* node;     ///< RDF node
    uint32_t   index;    ///< lv2:index
    LilvNode* symbol;   ///< lv2:symbol
    LilvNodes* classes;  ///< rdf:type
};

struct LilvSpecImpl {
    SordNode* spec;
    SordNode* bundle;
    LilvNodes* data_uris;
    struct LilvSpecImpl* next;
};

/**
   Header of an LilvPlugin, LilvPluginClass, or LilvUI.
   Any of these structs may be safely casted to LilvHeader, which is used to
   implement collections using the same comparator.
*/
struct LilvHeader {
    LilvWorld* world;
    LilvNode* uri;
};

#ifdef LILV_DYN_MANIFEST
typedef struct {
    LilvNode* bundle;
    void* lib;
    LV2_Dyn_Manifest_Handle handle;
    uint32_t                refs;
} LilvDynManifest;
#endif

typedef struct {
    LilvWorld* world;
    LilvNode* uri;
    char* bundle_path;
    void* lib;
    LV2_Descriptor_Function   lv2_descriptor;
    const LV2_Lib_Descriptor* desc;
    uint32_t                  refs;
} LilvLib;

struct LilvPluginImpl {
    LilvWorld* world;
    LilvNode* plugin_uri;
    LilvNode* bundle_uri;  ///< Bundle plugin was loaded from
    LilvNode* binary_uri;  ///< lv2:binary
#ifdef LILV_DYN_MANIFEST
    LilvDynManifest* dynmanifest;
#endif
    const LilvPluginClass* plugin_class;
    LilvNodes* data_uris;  ///< rdfs::seeAlso
    LilvPort** ports;
    uint32_t               num_ports;
    bool                   loaded;
    bool                   parse_errors;
    bool                   replaced;
};

struct LilvPluginClassImpl {
    LilvWorld* world;
    LilvNode* uri;
    LilvNode* parent_uri;
    LilvNode* label;
};

struct LilvInstancePimpl {
    LilvWorld* world;
    LilvLib* lib;
};

typedef struct {
    bool  dyn_manifest;
    bool  filter_language;
    char* lv2_path;
} LilvOptions;

struct LilvWorldImpl {
    SordWorld* world;
    SordModel* model;
    SerdReader* reader;
    unsigned           n_read_files;
    LilvPluginClass* lv2_plugin_class;
    LilvPluginClasses* plugin_classes;
    LilvSpec* specs;
    LilvPlugins* plugins;
    LilvPlugins* zombies;
    LilvNodes* loaded_files;
    ZixTree* libs;
    struct {
        SordNode* dc_replaces;
        SordNode* dman_DynManifest;
        SordNode* doap_name;
        SordNode* lv2_Plugin;
        SordNode* lv2_Specification;
        SordNode* lv2_appliesTo;
        SordNode* lv2_binary;
        SordNode* lv2_default;
        SordNode* lv2_designation;
        SordNode* lv2_extensionData;
        SordNode* lv2_index;
        SordNode* lv2_latency;
        SordNode* lv2_maximum;
        SordNode* lv2_microVersion;
        SordNode* lv2_minimum;
        SordNode* lv2_minorVersion;
        SordNode* lv2_name;
        SordNode* lv2_optionalFeature;
        SordNode* lv2_port;
        SordNode* lv2_portProperty;
        SordNode* lv2_reportsLatency;
        SordNode* lv2_requiredFeature;
        SordNode* lv2_symbol;
        SordNode* lv2_prototype;
        SordNode* owl_Ontology;
        SordNode* pset_value;
        SordNode* rdf_a;
        SordNode* rdf_value;
        SordNode* rdfs_Class;
        SordNode* rdfs_label;
        SordNode* rdfs_seeAlso;
        SordNode* rdfs_subClassOf;
        SordNode* xsd_base64Binary;
        SordNode* xsd_boolean;
        SordNode* xsd_decimal;
        SordNode* xsd_double;
        SordNode* xsd_integer;
        SordNode* null_uri;
    } uris;
    LilvOptions opt;
};

typedef enum {
    LILV_VALUE_URI,
    LILV_VALUE_STRING,
    LILV_VALUE_INT,
    LILV_VALUE_FLOAT,
    LILV_VALUE_BOOL,
    LILV_VALUE_BLANK,
    LILV_VALUE_BLOB
} LilvNodeType;

struct LilvNodeImpl {
    LilvWorld* world;
    SordNode* node;
    LilvNodeType type;
    union {
        int   int_val;
        float float_val;
        bool  bool_val;
    } val;
};

struct LilvScalePointImpl {
    LilvNode* value;
    LilvNode* label;
};

struct LilvUIImpl {
    LilvWorld* world;
    LilvNode* uri;
    LilvNode* bundle_uri;
    LilvNode* binary_uri;
    LilvNodes* classes;
};

typedef struct LilvVersion {
    int minor;
    int micro;
} LilvVersion;

/*
 *
 * Functions
 *
 */

LilvPort* lilv_port_new(LilvWorld* world,
    const SordNode* node,
    uint32_t        index,
    const char* symbol);
void      lilv_port_free(const LilvPlugin* plugin, LilvPort* port);

LilvPlugin* lilv_plugin_new(LilvWorld* world,
    LilvNode* uri,
    LilvNode* bundle_uri);
void        lilv_plugin_clear(LilvPlugin* plugin, LilvNode* bundle_uri);
void        lilv_plugin_load_if_necessary(const LilvPlugin* plugin);
void        lilv_plugin_free(LilvPlugin* plugin);
LilvNode* lilv_plugin_get_unique(const LilvPlugin* plugin,
    const SordNode* subject,
    const SordNode* predicate);

void      lilv_collection_free(LilvCollection* collection);
unsigned  lilv_collection_size(const LilvCollection* collection);
LilvIter* lilv_collection_begin(const LilvCollection* collection);
void* lilv_collection_get(const LilvCollection* collection,
    const LilvIter* i);

LilvPluginClass* lilv_plugin_class_new(LilvWorld* world,
    const SordNode* parent_node,
    const SordNode* uri,
    const char* label);

void lilv_plugin_class_free(LilvPluginClass* plugin_class);

LilvLib*
lilv_lib_open(LilvWorld* world,
    const LilvNode* uri,
    const char* bundle_path,
    const LV2_Feature* const* features);

const LV2_Descriptor* lilv_lib_get_plugin(LilvLib* lib, uint32_t index);
void                  lilv_lib_close(LilvLib* lib);

LilvNodes* lilv_nodes_new(void);
LilvPlugins* lilv_plugins_new(void);
LilvScalePoints* lilv_scale_points_new(void);
LilvPluginClasses* lilv_plugin_classes_new(void);
LilvUIs* lilv_uis_new(void);

LilvNode* lilv_world_get_manifest_uri(LilvWorld* world,
    const LilvNode* bundle_uri);

const uint8_t* lilv_world_blank_node_prefix(LilvWorld* world);

SerdStatus lilv_world_load_file(LilvWorld* world,
    SerdReader* reader,
    const LilvNode* uri);

SerdStatus
lilv_world_load_graph(LilvWorld* world,
    SordNode* graph,
    const LilvNode* uri);

LilvUI* lilv_ui_new(LilvWorld* world,
    LilvNode* uri,
    LilvNode* type_uri,
    LilvNode* binary_uri);

void lilv_ui_free(LilvUI* ui);

LilvNode* lilv_node_new(LilvWorld* world, LilvNodeType type, const char* str);
LilvNode* lilv_node_new_from_node(LilvWorld* world, const SordNode* node);

int lilv_header_compare_by_uri(const void* a, const void* b, void* user_data);
int lilv_lib_compare(const void* a, const void* b, void* user_data);

int lilv_ptr_cmp(const void* a, const void* b, void* user_data);
int lilv_resource_node_cmp(const void* a, const void* b, void* user_data);

static inline int
lilv_version_cmp(const LilvVersion* a, const LilvVersion* b)
{
    if (a->minor == b->minor && a->micro == b->micro) {
        return 0;
    }
    else if ((a->minor < b->minor)
        || (a->minor == b->minor && a->micro < b->micro)) {
        return -1;
    }
    else {
        return 1;
    }
}

struct LilvHeader*
    lilv_collection_get_by_uri(const ZixTree* seq, const LilvNode* uri);

LilvScalePoint* lilv_scale_point_new(LilvNode* value, LilvNode* label);
void            lilv_scale_point_free(LilvScalePoint* point);

SordIter*
lilv_world_query_internal(LilvWorld* world,
    const SordNode* subject,
    const SordNode* predicate,
    const SordNode* object);

bool
lilv_world_ask_internal(LilvWorld* world,
    const SordNode* subject,
    const SordNode* predicate,
    const SordNode* object);

LilvNodes*
lilv_world_find_nodes_internal(LilvWorld* world,
    const SordNode* subject,
    const SordNode* predicate,
    const SordNode* object);

SordModel*
lilv_world_filter_model(LilvWorld* world,
    SordModel* model,
    const SordNode* subject,
    const SordNode* predicate,
    const SordNode* object,
    const SordNode* graph);

#define FOREACH_MATCH(iter) \
	for (; !sord_iter_end(iter); sord_iter_next(iter))

LilvNodes* lilv_nodes_from_stream_objects(LilvWorld* world,
    SordIter* stream,
    SordQuadIndex field);

char* lilv_strjoin(const char* first, ...);
char* lilv_strdup(const char* str);
char* lilv_get_lang(void);
char* lilv_expand(const char* path);
char* lilv_dirname(const char* path);
char* lilv_dir_path(const char* path);
int    lilv_copy_file(const char* src, const char* dst);
bool   lilv_path_exists(const char* path, const void* ignored);
char* lilv_path_absolute(const char* path);
bool   lilv_path_is_absolute(const char* path);
char* lilv_get_latest_copy(const char* path, const char* copy_path);
char* lilv_path_relative_to(const char* path, const char* base);
bool   lilv_path_is_child(const char* path, const char* dir);
int    lilv_flock(FILE* file, bool lock);
char* lilv_realpath(const char* path);
int    lilv_symlink(const char* oldpath, const char* newpath);
int    lilv_mkdir_p(const char* dir_path);
char* lilv_path_join(const char* a, const char* b);
bool   lilv_file_equals(const char* a_path, const char* b_path);

char*
lilv_find_free_path(const char* in_path,
    bool (*exists)(const char*, const void*),
    const void* user_data);

void
lilv_dir_for_each(const char* path,
    void* data,
    void (*f)(const char* path, const char* name, void* data));

typedef void (*LilvVoidFunc)(void);

/** dlsym wrapper to return a function pointer (without annoying warning) */
static inline LilvVoidFunc
lilv_dlfunc(void* handle, const char* symbol)
{
#ifdef _WIN32
    return (LilvVoidFunc)GetProcAddress((HMODULE)handle, symbol);
#else
    typedef LilvVoidFunc(*VoidFuncGetter)(void*, const char*);
    VoidFuncGetter dlfunc = (VoidFuncGetter)dlsym;
    return dlfunc(handle, symbol);
#endif
}

#ifdef LILV_DYN_MANIFEST
static const LV2_Feature* const dman_features = { NULL };

void lilv_dynmanifest_free(LilvDynManifest* dynmanifest);
#endif

#define LILV_ERROR(str)       fprintf(stderr, "%s(): error: " str, \
                                      __func__)
#define LILV_ERRORF(fmt, ...) fprintf(stderr, "%s(): error: " fmt, \
                                      __func__, __VA_ARGS__)
#define LILV_WARN(str)        fprintf(stderr, "%s(): warning: " str, \
                                      __func__)
#define LILV_WARNF(fmt, ...)  fprintf(stderr, "%s(): warning: " fmt, \
                                      __func__, __VA_ARGS__)
#define LILV_NOTE(str)        fprintf(stderr, "%s(): note: " str, \
                                      __func__)
#define LILV_NOTEF(fmt, ...)  fprintf(stderr, "%s(): note: " fmt, \
                                      __func__, __VA_ARGS__)



int
lilv_ptr_cmp(const void* a, const void* b, void* user_data)
{
    return (intptr_t)a - (intptr_t)b;
}

int
lilv_resource_node_cmp(const void* a, const void* b, void* user_data)
{
    const SordNode* an = ((const LilvNode*)a)->node;
    const SordNode* bn = ((const LilvNode*)b)->node;
    return (intptr_t)an - (intptr_t)bn;
}

/* Generic collection functions */

static inline LilvCollection*
lilv_collection_new(ZixComparator cmp, ZixDestroyFunc destructor)
{
    return zix_tree_new(false, cmp, NULL, destructor);
}

void
lilv_collection_free(LilvCollection* collection)
{
    if (collection) {
        zix_tree_free((ZixTree*)collection);
    }
}

unsigned
lilv_collection_size(const LilvCollection* collection)
{
    return (collection ? zix_tree_size((const ZixTree*)collection) : 0);
}

LilvIter*
lilv_collection_begin(const LilvCollection* collection)
{
    return collection ? (LilvIter*)zix_tree_begin((ZixTree*)collection) : NULL;
}

void*
lilv_collection_get(const LilvCollection* collection,
    const LilvIter* i)
{
    return zix_tree_get((const ZixTreeIter*)i);
}

/* Constructors */

LilvScalePoints*
lilv_scale_points_new(void)
{
    return lilv_collection_new(lilv_ptr_cmp,
        (ZixDestroyFunc)lilv_scale_point_free);
}

LilvNodes*
lilv_nodes_new(void)
{
    return lilv_collection_new(lilv_ptr_cmp,
        (ZixDestroyFunc)lilv_node_free);
}

LilvUIs*
lilv_uis_new(void)
{
    return lilv_collection_new(lilv_header_compare_by_uri,
        (ZixDestroyFunc)lilv_ui_free);
}

LilvPluginClasses*
lilv_plugin_classes_new(void)
{
    return lilv_collection_new(lilv_header_compare_by_uri,
        (ZixDestroyFunc)lilv_plugin_class_free);
}

/* URI based accessors (for collections of things with URIs) */

const LilvPluginClass*
lilv_plugin_classes_get_by_uri(const LilvPluginClasses* classes,
    const LilvNode* uri)
{
    return (LilvPluginClass*)lilv_collection_get_by_uri(
        (const ZixTree*)classes, uri);
}

const LilvUI*
lilv_uis_get_by_uri(const LilvUIs* uis, const LilvNode* uri)
{
    return (LilvUI*)lilv_collection_get_by_uri((const ZixTree*)uis, uri);
}

/* Plugins */

LilvPlugins*
lilv_plugins_new(void)
{
    return lilv_collection_new(lilv_header_compare_by_uri, NULL);
}

const LilvPlugin*
lilv_plugins_get_by_uri(const LilvPlugins* plugins, const LilvNode* uri)
{
    return (LilvPlugin*)lilv_collection_get_by_uri(
        (const ZixTree*)plugins, uri);
}

/* Nodes */

bool
lilv_nodes_contains(const LilvNodes* nodes, const LilvNode* value)
{
    LILV_FOREACH(nodes, i, nodes) {
        if (lilv_node_equals(lilv_nodes_get(nodes, i), value)) {
            return true;
        }
    }

    return false;
}

LilvNodes*
lilv_nodes_merge(const LilvNodes* a, const LilvNodes* b)
{
    LilvNodes* result = lilv_nodes_new();

    LILV_FOREACH(nodes, i, a)
        zix_tree_insert((ZixTree*)result,
            lilv_node_duplicate(lilv_nodes_get(a, i)),
            NULL);

    LILV_FOREACH(nodes, i, b)
        zix_tree_insert((ZixTree*)result,
            lilv_node_duplicate(lilv_nodes_get(b, i)),
            NULL);

    return result;
}

/* Iterator */

#define LILV_COLLECTION_IMPL(prefix, CT, ET) \
\
unsigned \
prefix##_size(const CT* collection) { \
	return lilv_collection_size(collection); \
} \
\
\
LilvIter* \
prefix##_begin(const CT* collection) { \
	return lilv_collection_begin(collection); \
} \
\
\
const ET* \
prefix##_get(const CT* collection, LilvIter* i) { \
	return (ET*)lilv_collection_get(collection, i); \
} \
\
\
LilvIter* \
prefix##_next(const CT* collection, LilvIter* i) { \
	return zix_tree_iter_next((ZixTreeIter*)i); \
} \
\
\
bool \
prefix##_is_end(const CT* collection, LilvIter* i) { \
	return zix_tree_iter_is_end((ZixTreeIter*)i); \
}

LILV_COLLECTION_IMPL(lilv_plugin_classes, LilvPluginClasses, LilvPluginClass)
LILV_COLLECTION_IMPL(lilv_scale_points, LilvScalePoints, LilvScalePoint)
LILV_COLLECTION_IMPL(lilv_uis, LilvUIs, LilvUI)
LILV_COLLECTION_IMPL(lilv_nodes, LilvNodes, LilvNode)
LILV_COLLECTION_IMPL(lilv_plugins, LilvPlugins, LilvPlugin)

void
lilv_plugin_classes_free(LilvPluginClasses* collection) {
    lilv_collection_free(collection);
}

void
lilv_scale_points_free(LilvScalePoints* collection) {
    lilv_collection_free(collection);
}

void
lilv_uis_free(LilvUIs* collection) {
    lilv_collection_free(collection);
}

void
lilv_nodes_free(LilvNodes* collection) {
    lilv_collection_free(collection);
}

LilvNode*
lilv_nodes_get_first(const LilvNodes* collection) {
    return (LilvNode*)lilv_collection_get(collection,
        lilv_collection_begin(collection));
}

LilvInstance*
lilv_plugin_instantiate(const LilvPlugin* plugin,
    double                   sample_rate,
    const LV2_Feature* const* features)
{
    lilv_plugin_load_if_necessary(plugin);
    if (plugin->parse_errors) {
        return NULL;
    }

    LilvInstance* result = NULL;
    const LilvNode* const lib_uri = lilv_plugin_get_library_uri(plugin);
    const LilvNode* const bundle_uri = lilv_plugin_get_bundle_uri(plugin);
    if (!lib_uri || !bundle_uri) {
        return NULL;
    }

    char* const bundle_path = lilv_file_uri_parse(
        lilv_node_as_uri(bundle_uri), NULL);

    LilvLib* lib = lilv_lib_open(plugin->world, lib_uri, bundle_path, features);
    if (!lib) {
        serd_free(bundle_path);
        return NULL;
    }

    const LV2_Feature** local_features = NULL;
    if (features == NULL) {
        local_features = (const LV2_Feature**)malloc(sizeof(LV2_Feature*));
        local_features[0] = NULL;
    }

    // Search for plugin by URI
    for (uint32_t i = 0; true; ++i) {
        const LV2_Descriptor* ld = lilv_lib_get_plugin(lib, i);
        if (!ld) {
            LILV_ERRORF("No plugin <%s> in <%s>\n",
                lilv_node_as_uri(lilv_plugin_get_uri(plugin)),
                lilv_node_as_uri(lib_uri));
            lilv_lib_close(lib);
            break;  // return NULL
        }

        if (!strcmp(ld->URI, lilv_node_as_uri(lilv_plugin_get_uri(plugin)))) {
            // Create LilvInstance to return
            result = (LilvInstance*)malloc(sizeof(LilvInstance));
            result->lv2_descriptor = ld;
            result->lv2_handle = ld->instantiate(
                ld, sample_rate, bundle_path,
                (features) ? features : local_features);
            result->pimpl = lib;
            break;
        }
    }

    free(local_features);
    serd_free(bundle_path);

    if (result) {
        if (result->lv2_handle == NULL) {
            // Failed to instantiate
            free(result);
            lilv_lib_close(lib);
            return NULL;
        }

        // "Connect" all ports to NULL (catches bugs)
        for (uint32_t i = 0; i < lilv_plugin_get_num_ports(plugin); ++i) {
            result->lv2_descriptor->connect_port(result->lv2_handle, i, NULL);
        }
    }

    return result;
}

void
lilv_instance_free(LilvInstance* instance)
{
    if (!instance) {
        return;
    }

    instance->lv2_descriptor->cleanup(instance->lv2_handle);
    instance->lv2_descriptor = NULL;
    lilv_lib_close((LilvLib*)instance->pimpl);
    instance->pimpl = NULL;
    free(instance);
}


LilvLib*
lilv_lib_open(LilvWorld* world,
    const LilvNode* uri,
    const char* bundle_path,
    const LV2_Feature* const* features)
{
    ZixTreeIter* i = NULL;
    const LilvLib key = {
        world, (LilvNode*)uri, (char*)bundle_path, NULL, NULL, NULL, 0
    };
    if (!zix_tree_find(world->libs, &key, &i)) {
        LilvLib* llib = (LilvLib*)zix_tree_get(i);
        ++llib->refs;
        return llib;
    }

    const char* const lib_uri = lilv_node_as_uri(uri);
    char* const       lib_path = (char*)serd_file_uri_parse(
        (const uint8_t*)lib_uri, NULL);
    if (!lib_path) {
        return NULL;
    }

    dlerror();
    void* lib = dlopen(lib_path, RTLD_NOW);
    if (!lib) {
        LILV_ERRORF("Failed to open library %s (%s)\n", lib_path, dlerror());
        serd_free(lib_path);
        return NULL;
    }

    LV2_Descriptor_Function df = (LV2_Descriptor_Function)
        lilv_dlfunc(lib, "lv2_descriptor");

    LV2_Lib_Descriptor_Function ldf = (LV2_Lib_Descriptor_Function)
        lilv_dlfunc(lib, "lv2_lib_descriptor");

    const LV2_Lib_Descriptor* desc = NULL;
    if (ldf) {
        desc = ldf(bundle_path, features);
        if (!desc) {
            LILV_ERRORF("Call to %s:lv2_lib_descriptor failed\n", lib_path);
            dlclose(lib);
            serd_free(lib_path);
            return NULL;
        }
    }
    else if (!df) {
        LILV_ERRORF("No `lv2_descriptor' or `lv2_lib_descriptor' in %s\n",
            lib_path);
        dlclose(lib);
        serd_free(lib_path);
        return NULL;
    }
    serd_free(lib_path);

    LilvLib* llib = (LilvLib*)malloc(sizeof(LilvLib));
    llib->world = world;
    llib->uri = lilv_node_duplicate(uri);
    llib->bundle_path = lilv_strdup(bundle_path);
    llib->lib = lib;
    llib->lv2_descriptor = df;
    llib->desc = desc;
    llib->refs = 1;

    zix_tree_insert(world->libs, llib, NULL);
    return llib;
}

const LV2_Descriptor*
lilv_lib_get_plugin(LilvLib* lib, uint32_t index)
{
    if (lib->lv2_descriptor) {
        return lib->lv2_descriptor(index);
    }
    else if (lib->desc) {
        return lib->desc->get_plugin(lib->desc->handle, index);
    }
    return NULL;
}

void
lilv_lib_close(LilvLib* lib)
{
    if (--lib->refs == 0) {
        dlclose(lib->lib);

        ZixTreeIter* i = NULL;
        if (lib->world->libs && !zix_tree_find(lib->world->libs, lib, &i)) {
            zix_tree_remove(lib->world->libs, i);
        }

        lilv_node_free(lib->uri);
        free(lib->bundle_path);
        free(lib);
    }
}


static void
lilv_node_set_numerics_from_string(LilvNode* val)
{
    const char* str = (const char*)sord_node_get_string(val->node);

    switch (val->type) {
    case LILV_VALUE_URI:
    case LILV_VALUE_BLANK:
    case LILV_VALUE_STRING:
    case LILV_VALUE_BLOB:
        break;
    case LILV_VALUE_INT:
        val->val.int_val = strtol(str, NULL, 10);
        break;
    case LILV_VALUE_FLOAT:
        val->val.float_val = serd_strtod(str, NULL);
        break;
    case LILV_VALUE_BOOL:
        val->val.bool_val = !strcmp(str, "true");
        break;
    }
}

/** Note that if `type` is numeric or boolean, the returned value is corrupt
 * until lilv_node_set_numerics_from_string is called.  It is not
 * automatically called from here to avoid overhead and imprecision when the
 * exact string value is known.
 */
LilvNode*
lilv_node_new(LilvWorld* world, LilvNodeType type, const char* str)
{
    LilvNode* val = (LilvNode*)malloc(sizeof(LilvNode));
    val->world = world;
    val->type = type;

    const uint8_t* ustr = (const uint8_t*)str;
    switch (type) {
    case LILV_VALUE_URI:
        val->node = sord_new_uri(world->world, ustr);
        break;
    case LILV_VALUE_BLANK:
        val->node = sord_new_blank(world->world, ustr);
        break;
    case LILV_VALUE_STRING:
        val->node = sord_new_literal(world->world, NULL, ustr, NULL);
        break;
    case LILV_VALUE_INT:
        val->node = sord_new_literal(
            world->world, world->uris.xsd_integer, ustr, NULL);
        break;
    case LILV_VALUE_FLOAT:
        val->node = sord_new_literal(
            world->world, world->uris.xsd_decimal, ustr, NULL);
        break;
    case LILV_VALUE_BOOL:
        val->node = sord_new_literal(
            world->world, world->uris.xsd_boolean, ustr, NULL);
        break;
    case LILV_VALUE_BLOB:
        val->node = sord_new_literal(
            world->world, world->uris.xsd_base64Binary, ustr, NULL);
        break;
    }

    if (!val->node) {
        free(val);
        return NULL;
    }

    return val;
}

/** Create a new LilvNode from `node`, or return NULL if impossible */
LilvNode*
lilv_node_new_from_node(LilvWorld* world, const SordNode* node)
{
    if (!node) {
        return NULL;
    }

    LilvNode* result = NULL;
    SordNode* datatype_uri = NULL;
    LilvNodeType type = LILV_VALUE_STRING;

    switch (sord_node_get_type(node)) {
    case SORD_URI:
        result = (LilvNode*)malloc(sizeof(LilvNode));
        result->world = world;
        result->type = LILV_VALUE_URI;
        result->node = sord_node_copy(node);
        break;
    case SORD_BLANK:
        result = (LilvNode*)malloc(sizeof(LilvNode));
        result->world = world;
        result->type = LILV_VALUE_BLANK;
        result->node = sord_node_copy(node);
        break;
    case SORD_LITERAL:
        datatype_uri = sord_node_get_datatype(node);
        if (datatype_uri) {
            if (sord_node_equals(datatype_uri, world->uris.xsd_boolean)) {
                type = LILV_VALUE_BOOL;
            }
            else if (sord_node_equals(datatype_uri, world->uris.xsd_decimal) ||
                sord_node_equals(datatype_uri, world->uris.xsd_double)) {
                type = LILV_VALUE_FLOAT;
            }
            else if (sord_node_equals(datatype_uri, world->uris.xsd_integer)) {
                type = LILV_VALUE_INT;
            }
            else if (sord_node_equals(datatype_uri,
                world->uris.xsd_base64Binary)) {
                type = LILV_VALUE_BLOB;
            }
            else {
                LILV_ERRORF("Unknown datatype `%s'\n",
                    sord_node_get_string(datatype_uri));
            }
        }
        result = lilv_node_new(
            world, type, (const char*)sord_node_get_string(node));
        lilv_node_set_numerics_from_string(result);
        break;
    }

    return result;
}

LilvNode*
lilv_new_uri(LilvWorld* world, const char* uri)
{
    return lilv_node_new(world, LILV_VALUE_URI, uri);
}

LilvNode*
lilv_new_file_uri(LilvWorld* world, const char* host, const char* path)
{
    char* abs_path = lilv_path_absolute(path);
    SerdNode s = serd_node_new_file_uri(
        (const uint8_t*)abs_path, (const uint8_t*)host, NULL, true);

    LilvNode* ret = lilv_node_new(world, LILV_VALUE_URI, (const char*)s.buf);
    serd_node_free(&s);
    free(abs_path);
    return ret;
}

LilvNode*
lilv_new_string(LilvWorld* world, const char* str)
{
    return lilv_node_new(world, LILV_VALUE_STRING, str);
}

LilvNode*
lilv_new_int(LilvWorld* world, int val)
{
    char str[32];
    snprintf(str, sizeof(str), "%d", val);
    LilvNode* ret = lilv_node_new(world, LILV_VALUE_INT, str);
    ret->val.int_val = val;
    return ret;
}

LilvNode*
lilv_new_float(LilvWorld* world, float val)
{
    char str[32];
    snprintf(str, sizeof(str), "%f", val);
    LilvNode* ret = lilv_node_new(world, LILV_VALUE_FLOAT, str);
    ret->val.float_val = val;
    return ret;
}

LilvNode*
lilv_new_bool(LilvWorld* world, bool val)
{
    LilvNode* ret = lilv_node_new(world, LILV_VALUE_BOOL,
        val ? "true" : "false");
    ret->val.bool_val = val;
    return ret;
}

LilvNode*
lilv_node_duplicate(const LilvNode* val)
{
    if (!val) {
        return NULL;
    }

    LilvNode* result = (LilvNode*)malloc(sizeof(LilvNode));
    result->world = val->world;
    result->node = sord_node_copy(val->node);
    result->val = val->val;
    result->type = val->type;
    return result;
}

void
lilv_node_free(LilvNode* val)
{
    if (val) {
        sord_node_free(val->world->world, val->node);
        free(val);
    }
}

bool
lilv_node_equals(const LilvNode* value, const LilvNode* other)
{
    if (value == NULL && other == NULL) {
        return true;
    }
    else if (value == NULL || other == NULL) {
        return false;
    }
    else if (value->type != other->type) {
        return false;
    }

    switch (value->type) {
    case LILV_VALUE_URI:
    case LILV_VALUE_BLANK:
    case LILV_VALUE_STRING:
    case LILV_VALUE_BLOB:
        return sord_node_equals(value->node, other->node);
    case LILV_VALUE_INT:
        return (value->val.int_val == other->val.int_val);
    case LILV_VALUE_FLOAT:
        return (value->val.float_val == other->val.float_val);
    case LILV_VALUE_BOOL:
        return (value->val.bool_val == other->val.bool_val);
    }

    return false; /* shouldn't get here */
}

char*
lilv_node_get_turtle_token(const LilvNode* value)
{
    const char* str = (const char*)sord_node_get_string(value->node);
    size_t      len = 0;
    char* result = NULL;
    SerdNode    node;

    switch (value->type) {
    case LILV_VALUE_URI:
        len = strlen(str) + 3;
        result = (char*)calloc(len, 1);
        snprintf(result, len, "<%s>", str);
        break;
    case LILV_VALUE_BLANK:
        len = strlen(str) + 3;
        result = (char*)calloc(len, 1);
        snprintf(result, len, "_:%s", str);
        break;
    case LILV_VALUE_STRING:
    case LILV_VALUE_BOOL:
    case LILV_VALUE_BLOB:
        result = lilv_strdup(str);
        break;
    case LILV_VALUE_INT:
        node = serd_node_new_integer(value->val.int_val);
        result = lilv_strdup((char*)node.buf);
        serd_node_free(&node);
        break;
    case LILV_VALUE_FLOAT:
        node = serd_node_new_decimal(value->val.float_val, 8);
        result = lilv_strdup((char*)node.buf);
        serd_node_free(&node);
        break;
    }

    return result;
}

bool
lilv_node_is_uri(const LilvNode* value)
{
    return (value && value->type == LILV_VALUE_URI);
}

const char*
lilv_node_as_uri(const LilvNode* value)
{
    return (lilv_node_is_uri(value)
        ? (const char*)sord_node_get_string(value->node)
        : NULL);
}

bool
lilv_node_is_blank(const LilvNode* value)
{
    return (value && value->type == LILV_VALUE_BLANK);
}

const char*
lilv_node_as_blank(const LilvNode* value)
{
    return (lilv_node_is_blank(value)
        ? (const char*)sord_node_get_string(value->node)
        : NULL);
}

bool
lilv_node_is_literal(const LilvNode* value)
{
    if (!value) {
        return false;
    }

    switch (value->type) {
    case LILV_VALUE_STRING:
    case LILV_VALUE_INT:
    case LILV_VALUE_FLOAT:
    case LILV_VALUE_BLOB:
        return true;
    default:
        return false;
    }
}

bool
lilv_node_is_string(const LilvNode* value)
{
    return (value && value->type == LILV_VALUE_STRING);
}

const char*
lilv_node_as_string(const LilvNode* value)
{
    return value ? (const char*)sord_node_get_string(value->node) : NULL;
}

bool
lilv_node_is_int(const LilvNode* value)
{
    return (value && value->type == LILV_VALUE_INT);
}

int
lilv_node_as_int(const LilvNode* value)
{
    return lilv_node_is_int(value) ? value->val.int_val : 0;
}

bool
lilv_node_is_float(const LilvNode* value)
{
    return (value && value->type == LILV_VALUE_FLOAT);
}

float
lilv_node_as_float(const LilvNode* value)
{
    if (lilv_node_is_float(value)) {
        return value->val.float_val;
    }
    else if (lilv_node_is_int(value)) {
        return (float)value->val.int_val;
    }
    return NAN;
}

bool
lilv_node_is_bool(const LilvNode* value)
{
    return (value && value->type == LILV_VALUE_BOOL);
}

bool
lilv_node_as_bool(const LilvNode* value)
{
    return lilv_node_is_bool(value) ? value->val.bool_val : false;
}

char*
lilv_node_get_path(const LilvNode* value, char** hostname)
{
    if (lilv_node_is_uri(value)) {
        return lilv_file_uri_parse(lilv_node_as_uri(value), hostname);
    }
    return NULL;
}


#define NS_DOAP (const uint8_t*)"http://usefulinc.com/ns/doap#"
#define NS_FOAF (const uint8_t*)"http://xmlns.com/foaf/0.1/"

static void
lilv_plugin_init(LilvPlugin* plugin, LilvNode* bundle_uri)
{
    plugin->bundle_uri = bundle_uri;
    plugin->binary_uri = NULL;
#ifdef LILV_DYN_MANIFEST
    plugin->dynmanifest = NULL;
#endif
    plugin->plugin_class = NULL;
    plugin->data_uris = lilv_nodes_new();
    plugin->ports = NULL;
    plugin->num_ports = 0;
    plugin->loaded = false;
    plugin->parse_errors = false;
    plugin->replaced = false;
}

/** Ownership of `uri` and `bundle` is taken */
LilvPlugin*
lilv_plugin_new(LilvWorld* world, LilvNode* uri, LilvNode* bundle_uri)
{
    LilvPlugin* plugin = (LilvPlugin*)malloc(sizeof(LilvPlugin));

    plugin->world = world;
    plugin->plugin_uri = uri;

    lilv_plugin_init(plugin, bundle_uri);
    return plugin;
}

void
lilv_plugin_clear(LilvPlugin* plugin, LilvNode* bundle_uri)
{
    lilv_node_free(plugin->bundle_uri);
    lilv_node_free(plugin->binary_uri);
    lilv_nodes_free(plugin->data_uris);
    lilv_plugin_init(plugin, bundle_uri);
}

static void
lilv_plugin_free_ports(LilvPlugin* plugin)
{
    if (plugin->ports) {
        for (uint32_t i = 0; i < plugin->num_ports; ++i) {
            lilv_port_free(plugin, plugin->ports[i]);
        }
        free(plugin->ports);
        plugin->num_ports = 0;
        plugin->ports = NULL;
    }
}

void
lilv_plugin_free(LilvPlugin* plugin)
{
#ifdef LILV_DYN_MANIFEST
    if (plugin->dynmanifest && --plugin->dynmanifest->refs == 0) {
        lilv_dynmanifest_free(plugin->dynmanifest);
    }
#endif

    lilv_node_free(plugin->plugin_uri);
    plugin->plugin_uri = NULL;

    lilv_node_free(plugin->bundle_uri);
    plugin->bundle_uri = NULL;

    lilv_node_free(plugin->binary_uri);
    plugin->binary_uri = NULL;

    lilv_plugin_free_ports(plugin);

    lilv_nodes_free(plugin->data_uris);
    plugin->data_uris = NULL;

    free(plugin);
}

static LilvNode*
lilv_plugin_get_one(const LilvPlugin* plugin,
    const SordNode* subject,
    const SordNode* predicate)
{
    /* TODO: This is slower than it could be in some cases, but it's simpler to
       use the existing i18n code. */

    SordIter* stream =
        lilv_world_query_internal(plugin->world, subject, predicate, NULL);

    LilvNodes* nodes = lilv_nodes_from_stream_objects(
        plugin->world, stream, SORD_OBJECT);

    if (nodes) {
        LilvNode* value = lilv_node_duplicate(lilv_nodes_get_first(nodes));
        lilv_nodes_free(nodes);
        return value;
    }

    return NULL;
}

LilvNode*
lilv_plugin_get_unique(const LilvPlugin* plugin,
    const SordNode* subject,
    const SordNode* predicate)
{
    LilvNode* ret = lilv_plugin_get_one(plugin, subject, predicate);
    if (!ret) {
        LILV_ERRORF("No value found for (%s %s ...) property\n",
            sord_node_get_string(subject),
            sord_node_get_string(predicate));
    }
    return ret;
}

static void
lilv_plugin_load(LilvPlugin* plugin)
{
    SordNode* bundle_uri_node = plugin->bundle_uri->node;
    const SerdNode* bundle_uri_snode = sord_node_to_serd_node(bundle_uri_node);

    SerdEnv* env = serd_env_new(bundle_uri_snode);
    SerdReader* reader = sord_new_reader(plugin->world->model, env, SERD_TURTLE,
        bundle_uri_node);

    SordModel* prots = lilv_world_filter_model(
        plugin->world,
        plugin->world->model,
        plugin->plugin_uri->node,
        plugin->world->uris.lv2_prototype,
        NULL, NULL);
    SordModel* skel = sord_new(plugin->world->world, SORD_SPO, false);
    SordIter* iter = sord_begin(prots);
    for (; !sord_iter_end(iter); sord_iter_next(iter)) {
        const SordNode* t = sord_iter_get_node(iter, SORD_OBJECT);
        LilvNode* prototype = lilv_node_new_from_node(plugin->world, t);

        lilv_world_load_resource(plugin->world, prototype);

        SordIter* statements = sord_search(
            plugin->world->model, prototype->node, NULL, NULL, NULL);
        FOREACH_MATCH(statements) {
            SordQuad quad;
            sord_iter_get(statements, quad);
            quad[0] = plugin->plugin_uri->node;
            sord_add(skel, quad);
        }

        sord_iter_free(statements);
        lilv_node_free(prototype);
    }
    sord_iter_free(iter);

    for (iter = sord_begin(skel); !sord_iter_end(iter); sord_iter_next(iter)) {
        SordQuad quad;
        sord_iter_get(iter, quad);
        sord_add(plugin->world->model, quad);
    }
    sord_iter_free(iter);
    sord_free(skel);
    sord_free(prots);

    // Parse all the plugin's data files into RDF model
    SerdStatus st = SERD_SUCCESS;
    LILV_FOREACH(nodes, i, plugin->data_uris) {
        const LilvNode* data_uri = lilv_nodes_get(plugin->data_uris, i);

        serd_env_set_base_uri(env, sord_node_to_serd_node(data_uri->node));
        st = lilv_world_load_file(plugin->world, reader, data_uri);
        if (st > SERD_FAILURE) {
            break;
        }
    }

    if (st > SERD_FAILURE) {
        plugin->loaded = true;
        plugin->parse_errors = true;
        serd_reader_free(reader);
        serd_env_free(env);
        return;
    }

#ifdef LILV_DYN_MANIFEST
    // Load and parse dynamic manifest data, if this is a library
    if (plugin->dynmanifest) {
        typedef int (*GetDataFunc)(LV2_Dyn_Manifest_Handle handle,
            FILE* fp,
            const char* uri);
        GetDataFunc get_data_func = (GetDataFunc)lilv_dlfunc(
            plugin->dynmanifest->lib, "lv2_dyn_manifest_get_data");
        if (get_data_func) {
            const SordNode* bundle = plugin->dynmanifest->bundle->node;
            serd_env_set_base_uri(env, sord_node_to_serd_node(bundle));
            FILE* fd = tmpfile();
            get_data_func(plugin->dynmanifest->handle, fd,
                lilv_node_as_string(plugin->plugin_uri));
            rewind(fd);
            serd_reader_add_blank_prefix(
                reader, lilv_world_blank_node_prefix(plugin->world));
            serd_reader_read_file_handle(
                reader, fd, (const uint8_t*)"(dyn-manifest)");
            fclose(fd);
        }
    }
#endif
    serd_reader_free(reader);
    serd_env_free(env);

    plugin->loaded = true;
}

static bool
is_symbol(const char* str)
{
    for (const char* s = str; *s; ++s) {
        if (!((*s >= 'a' && *s <= 'z') ||
            (*s >= 'A' && *s <= 'Z') ||
            (s > str && *s >= '0' && *s <= '9') ||
            *s == '_')) {
            return false;
        }
    }
    return true;
}

static void
lilv_plugin_load_ports_if_necessary(const LilvPlugin* const_plugin)
{
    LilvPlugin* plugin = (LilvPlugin*)const_plugin;

    lilv_plugin_load_if_necessary(plugin);

    if (!plugin->ports) {
        plugin->ports = (LilvPort**)malloc(sizeof(LilvPort*));
        plugin->ports[0] = NULL;

        SordIter* ports = lilv_world_query_internal(
            plugin->world,
            plugin->plugin_uri->node,
            plugin->world->uris.lv2_port,
            NULL);

        FOREACH_MATCH(ports) {
            const SordNode* port = sord_iter_get_node(ports, SORD_OBJECT);
            LilvNode* index = lilv_plugin_get_unique(
                plugin, port, plugin->world->uris.lv2_index);
            LilvNode* symbol = lilv_plugin_get_unique(
                plugin, port, plugin->world->uris.lv2_symbol);

            if (!lilv_node_is_string(symbol) ||
                !is_symbol((const char*)sord_node_get_string(symbol->node))) {
                LILV_ERRORF("Plugin <%s> port symbol `%s' is invalid\n",
                    lilv_node_as_uri(plugin->plugin_uri),
                    lilv_node_as_string(symbol));
                lilv_node_free(symbol);
                lilv_node_free(index);
                lilv_plugin_free_ports(plugin);
                break;
            }

            if (!lilv_node_is_int(index)) {
                LILV_ERRORF("Plugin <%s> port index is not an integer\n",
                    lilv_node_as_uri(plugin->plugin_uri));
                lilv_node_free(symbol);
                lilv_node_free(index);
                lilv_plugin_free_ports(plugin);
                break;
            }

            uint32_t  this_index = lilv_node_as_int(index);
            LilvPort* this_port = NULL;
            if (plugin->num_ports > this_index) {
                this_port = plugin->ports[this_index];
            }
            else {
                plugin->ports = (LilvPort**)realloc(
                    plugin->ports, (this_index + 1) * sizeof(LilvPort*));
                memset(plugin->ports + plugin->num_ports, '\0',
                    (this_index - plugin->num_ports) * sizeof(LilvPort*));
                plugin->num_ports = this_index + 1;
            }

            // Havn't seen this port yet, add it to array
            if (!this_port) {
                this_port = lilv_port_new(plugin->world,
                    port,
                    this_index,
                    lilv_node_as_string(symbol));
                plugin->ports[this_index] = this_port;
            }

            SordIter* types = lilv_world_query_internal(
                plugin->world, port, plugin->world->uris.rdf_a, NULL);
            FOREACH_MATCH(types) {
                const SordNode* type = sord_iter_get_node(types, SORD_OBJECT);
                if (sord_node_get_type(type) == SORD_URI) {
                    zix_tree_insert(
                        (ZixTree*)this_port->classes,
                        lilv_node_new_from_node(plugin->world, type), NULL);
                }
                else {
                    LILV_WARNF("Plugin <%s> port type is not a URI\n",
                        lilv_node_as_uri(plugin->plugin_uri));
                }
            }
            sord_iter_free(types);

            lilv_node_free(symbol);
            lilv_node_free(index);
        }
        sord_iter_free(ports);

        // Check sanity
        for (uint32_t i = 0; i < plugin->num_ports; ++i) {
            if (!plugin->ports[i]) {
                LILV_ERRORF("Plugin <%s> is missing port %d/%d\n",
                    lilv_node_as_uri(plugin->plugin_uri), i, plugin->num_ports);
                lilv_plugin_free_ports(plugin);
                break;
            }
        }
    }
}

void
lilv_plugin_load_if_necessary(const LilvPlugin* plugin)
{
    if (!plugin->loaded) {
        lilv_plugin_load((LilvPlugin*)plugin);
    }
}

const LilvNode*
lilv_plugin_get_uri(const LilvPlugin* plugin)
{
    return plugin->plugin_uri;
}

const LilvNode*
lilv_plugin_get_bundle_uri(const LilvPlugin* plugin)
{
    return plugin->bundle_uri;
}

const LilvNode*
lilv_plugin_get_library_uri(const LilvPlugin* plugin)
{
    lilv_plugin_load_if_necessary((LilvPlugin*)plugin);
    if (!plugin->binary_uri) {
        // <plugin> lv2:binary ?binary
        SordIter* i = lilv_world_query_internal(plugin->world,
            plugin->plugin_uri->node,
            plugin->world->uris.lv2_binary,
            NULL);
        FOREACH_MATCH(i) {
            const SordNode* binary_node = sord_iter_get_node(i, SORD_OBJECT);
            if (sord_node_get_type(binary_node) == SORD_URI) {
                ((LilvPlugin*)plugin)->binary_uri =
                    lilv_node_new_from_node(plugin->world, binary_node);
                break;
            }
        }
        sord_iter_free(i);
    }
    if (!plugin->binary_uri) {
        LILV_WARNF("Plugin <%s> has no lv2:binary\n",
            lilv_node_as_uri(lilv_plugin_get_uri(plugin)));
    }
    return plugin->binary_uri;
}

const LilvNodes*
lilv_plugin_get_data_uris(const LilvPlugin* plugin)
{
    return plugin->data_uris;
}

const LilvPluginClass*
lilv_plugin_get_class(const LilvPlugin* plugin)
{
    lilv_plugin_load_if_necessary((LilvPlugin*)plugin);
    if (!plugin->plugin_class) {
        // <plugin> a ?class
        SordIter* c = lilv_world_query_internal(plugin->world,
            plugin->plugin_uri->node,
            plugin->world->uris.rdf_a,
            NULL);
        FOREACH_MATCH(c) {
            const SordNode* class_node = sord_iter_get_node(c, SORD_OBJECT);
            if (sord_node_get_type(class_node) != SORD_URI) {
                continue;
            }

            LilvNode* klass = lilv_node_new_from_node(plugin->world, class_node);
            if (!lilv_node_equals(klass, plugin->world->lv2_plugin_class->uri)) {
                const LilvPluginClass* pclass = lilv_plugin_classes_get_by_uri(
                    plugin->world->plugin_classes, klass);

                if (pclass) {
                    ((LilvPlugin*)plugin)->plugin_class = pclass;
                    lilv_node_free(klass);
                    break;
                }
            }

            lilv_node_free(klass);
        }
        sord_iter_free(c);

        if (plugin->plugin_class == NULL) {
            ((LilvPlugin*)plugin)->plugin_class =
                plugin->world->lv2_plugin_class;
        }
    }
    return plugin->plugin_class;
}

static LilvNodes*
lilv_plugin_get_value_internal(const LilvPlugin* plugin,
    const SordNode* predicate)
{
    lilv_plugin_load_if_necessary(plugin);
    return lilv_world_find_nodes_internal(
        plugin->world, plugin->plugin_uri->node, predicate, NULL);
}

bool
lilv_plugin_verify(const LilvPlugin* plugin)
{
    lilv_plugin_load_if_necessary(plugin);
    if (plugin->parse_errors) {
        return false;
    }

    LilvNode* rdf_type = lilv_new_uri(plugin->world, LILV_NS_RDF "type");
    LilvNodes* results = lilv_plugin_get_value(plugin, rdf_type);
    lilv_node_free(rdf_type);
    if (!results) {
        return false;
    }

    lilv_nodes_free(results);
    results = lilv_plugin_get_value_internal(plugin,
        plugin->world->uris.doap_name);
    if (!results) {
        return false;
    }

    lilv_nodes_free(results);
    LilvNode* lv2_port = lilv_new_uri(plugin->world, LV2_CORE__port);
    results = lilv_plugin_get_value(plugin, lv2_port);
    lilv_node_free(lv2_port);
    if (!results) {
        return false;
    }

    lilv_nodes_free(results);
    return true;
}

LilvNode*
lilv_plugin_get_name(const LilvPlugin* plugin)
{
    LilvNodes* results = lilv_plugin_get_value_internal(
        plugin, plugin->world->uris.doap_name);

    LilvNode* ret = NULL;
    if (results) {
        LilvNode* val = lilv_nodes_get_first(results);
        if (lilv_node_is_string(val)) {
            ret = lilv_node_duplicate(val);
        }
        lilv_nodes_free(results);
    }

    if (!ret) {
        LILV_WARNF("Plugin <%s> has no (mandatory) doap:name\n",
            lilv_node_as_string(lilv_plugin_get_uri(plugin)));
    }

    return ret;
}

LilvNodes*
lilv_plugin_get_value(const LilvPlugin* plugin,
    const LilvNode* predicate)
{
    lilv_plugin_load_if_necessary(plugin);
    return lilv_world_find_nodes(plugin->world, plugin->plugin_uri, predicate, NULL);
}

uint32_t
lilv_plugin_get_num_ports(const LilvPlugin* plugin)
{
    lilv_plugin_load_ports_if_necessary(plugin);
    return plugin->num_ports;
}

void
lilv_plugin_get_port_ranges_float(const LilvPlugin* plugin,
    float* min_values,
    float* max_values,
    float* def_values)
{
    lilv_plugin_load_ports_if_necessary(plugin);
    LilvNode* min = NULL;
    LilvNode* max = NULL;
    LilvNode* def = NULL;
    LilvNode** minptr = min_values ? &min : NULL;
    LilvNode** maxptr = max_values ? &max : NULL;
    LilvNode** defptr = def_values ? &def : NULL;

    for (uint32_t i = 0; i < plugin->num_ports; ++i) {
        lilv_port_get_range(plugin, plugin->ports[i], defptr, minptr, maxptr);

        if (min_values) {
            if (lilv_node_is_float(min) || lilv_node_is_int(min)) {
                min_values[i] = lilv_node_as_float(min);
            }
            else {
                min_values[i] = NAN;
            }
        }

        if (max_values) {
            if (lilv_node_is_float(max) || lilv_node_is_int(max)) {
                max_values[i] = lilv_node_as_float(max);
            }
            else {
                max_values[i] = NAN;
            }
        }

        if (def_values) {
            if (lilv_node_is_float(def) || lilv_node_is_int(def)) {
                def_values[i] = lilv_node_as_float(def);
            }
            else {
                def_values[i] = NAN;
            }
        }

        lilv_node_free(def);
        lilv_node_free(min);
        lilv_node_free(max);
    }
}

uint32_t
lilv_plugin_get_num_ports_of_class_va(const LilvPlugin* plugin,
    const LilvNode* class_1,
    va_list           args)
{
    lilv_plugin_load_ports_if_necessary(plugin);

    uint32_t count = 0;

    // Build array of classes from args so we can walk it several times
    size_t           n_classes = 0;
    const LilvNode** classes = NULL;
    for (LilvNode* c = NULL; (c = va_arg(args, LilvNode*)); ) {
        classes = (const LilvNode**)realloc(
            classes, ++n_classes * sizeof(LilvNode*));
        classes[n_classes - 1] = c;
    }

    // Check each port against every type
    for (unsigned i = 0; i < plugin->num_ports; ++i) {
        LilvPort* port = plugin->ports[i];
        if (port && lilv_port_is_a(plugin, port, class_1)) {
            bool matches = true;
            for (size_t j = 0; j < n_classes; ++j) {
                if (!lilv_port_is_a(plugin, port, classes[j])) {
                    matches = false;
                    break;
                }
            }

            if (matches) {
                ++count;
            }
        }
    }

    free(classes);
    return count;
}

uint32_t
lilv_plugin_get_num_ports_of_class(const LilvPlugin* plugin,
    const LilvNode* class_1, ...)
{
    va_list args;
    va_start(args, class_1);

    uint32_t count = lilv_plugin_get_num_ports_of_class_va(plugin, class_1, args);

    va_end(args);
    return count;
}

bool
lilv_plugin_has_latency(const LilvPlugin* plugin)
{
    lilv_plugin_load_if_necessary(plugin);
    SordIter* ports = lilv_world_query_internal(
        plugin->world,
        plugin->plugin_uri->node,
        plugin->world->uris.lv2_port,
        NULL);

    bool ret = false;
    FOREACH_MATCH(ports) {
        const SordNode* port = sord_iter_get_node(ports, SORD_OBJECT);
        SordIter* prop = lilv_world_query_internal(
            plugin->world,
            port,
            plugin->world->uris.lv2_portProperty,
            plugin->world->uris.lv2_reportsLatency);
        SordIter* des = lilv_world_query_internal(
            plugin->world,
            port,
            plugin->world->uris.lv2_designation,
            plugin->world->uris.lv2_latency);
        const bool latent = !sord_iter_end(prop) || !sord_iter_end(des);
        sord_iter_free(prop);
        sord_iter_free(des);
        if (latent) {
            ret = true;
            break;
        }
    }
    sord_iter_free(ports);

    return ret;
}

static const LilvPort*
lilv_plugin_get_port_by_property(const LilvPlugin* plugin,
    const SordNode* port_property)
{
    lilv_plugin_load_ports_if_necessary(plugin);
    for (uint32_t i = 0; i < plugin->num_ports; ++i) {
        LilvPort* port = plugin->ports[i];
        SordIter* iter = lilv_world_query_internal(
            plugin->world,
            port->node->node,
            plugin->world->uris.lv2_portProperty,
            port_property);

        const bool found = !sord_iter_end(iter);
        sord_iter_free(iter);

        if (found) {
            return port;
        }
    }

    return NULL;
}

const LilvPort*
lilv_plugin_get_port_by_designation(const LilvPlugin* plugin,
    const LilvNode* port_class,
    const LilvNode* designation)
{
    LilvWorld* world = plugin->world;
    lilv_plugin_load_ports_if_necessary(plugin);
    for (uint32_t i = 0; i < plugin->num_ports; ++i) {
        LilvPort* port = plugin->ports[i];
        SordIter* iter = lilv_world_query_internal(
            world,
            port->node->node,
            world->uris.lv2_designation,
            designation->node);

        const bool found = !sord_iter_end(iter) &&
            (!port_class || lilv_port_is_a(plugin, port, port_class));
        sord_iter_free(iter);

        if (found) {
            return port;
        }
    }

    return NULL;
}

uint32_t
lilv_plugin_get_latency_port_index(const LilvPlugin* plugin)
{
    LilvNode* lv2_OutputPort =
        lilv_new_uri(plugin->world, LV2_CORE__OutputPort);
    LilvNode* lv2_latency =
        lilv_new_uri(plugin->world, LV2_CORE__latency);

    const LilvPort* prop_port = lilv_plugin_get_port_by_property(
        plugin, plugin->world->uris.lv2_reportsLatency);
    const LilvPort* des_port = lilv_plugin_get_port_by_designation(
        plugin, lv2_OutputPort, lv2_latency);

    lilv_node_free(lv2_latency);
    lilv_node_free(lv2_OutputPort);

    if (prop_port) {
        return prop_port->index;
    }
    else if (des_port) {
        return des_port->index;
    }
    else {
        return (uint32_t)-1;
    }
}

bool
lilv_plugin_has_feature(const LilvPlugin* plugin,
    const LilvNode* feature)
{
    lilv_plugin_load_if_necessary(plugin);
    const SordNode* predicates[] = { plugin->world->uris.lv2_requiredFeature,
                                     plugin->world->uris.lv2_optionalFeature,
                                     NULL };

    for (const SordNode** pred = predicates; *pred; ++pred) {
        if (lilv_world_ask_internal(
            plugin->world, plugin->plugin_uri->node, *pred, feature->node)) {
            return true;
        }
    }
    return false;
}

LilvNodes*
lilv_plugin_get_supported_features(const LilvPlugin* plugin)
{
    LilvNodes* optional = lilv_plugin_get_optional_features(plugin);
    LilvNodes* required = lilv_plugin_get_required_features(plugin);
    LilvNodes* result = lilv_nodes_merge(optional, required);
    lilv_nodes_free(optional);
    lilv_nodes_free(required);
    return result;
}

LilvNodes*
lilv_plugin_get_optional_features(const LilvPlugin* plugin)
{
    lilv_plugin_load_if_necessary(plugin);
    return lilv_world_find_nodes_internal(plugin->world,
        plugin->plugin_uri->node,
        plugin->world->uris.lv2_optionalFeature,
        NULL);
}

LilvNodes*
lilv_plugin_get_required_features(const LilvPlugin* plugin)
{
    lilv_plugin_load_if_necessary(plugin);
    return lilv_world_find_nodes_internal(plugin->world,
        plugin->plugin_uri->node,
        plugin->world->uris.lv2_requiredFeature,
        NULL);
}

bool
lilv_plugin_has_extension_data(const LilvPlugin* plugin,
    const LilvNode* uri)
{
    if (!lilv_node_is_uri(uri)) {
        LILV_ERRORF("Extension data `%s' is not a URI\n",
            sord_node_get_string(uri->node));
        return false;
    }

    lilv_plugin_load_if_necessary(plugin);
    return lilv_world_ask_internal(
        plugin->world,
        plugin->plugin_uri->node,
        plugin->world->uris.lv2_extensionData,
        uri->node);
}

LilvNodes*
lilv_plugin_get_extension_data(const LilvPlugin* plugin)
{
    return lilv_plugin_get_value_internal(plugin, plugin->world->uris.lv2_extensionData);
}

const LilvPort*
lilv_plugin_get_port_by_index(const LilvPlugin* plugin,
    uint32_t          index)
{
    lilv_plugin_load_ports_if_necessary(plugin);
    if (index < plugin->num_ports) {
        return plugin->ports[index];
    }
    else {
        return NULL;
    }
}

const LilvPort*
lilv_plugin_get_port_by_symbol(const LilvPlugin* plugin,
    const LilvNode* symbol)
{
    lilv_plugin_load_ports_if_necessary(plugin);
    for (uint32_t i = 0; i < plugin->num_ports; ++i) {
        LilvPort* port = plugin->ports[i];
        if (lilv_node_equals(port->symbol, symbol)) {
            return port;
        }
    }

    return NULL;
}

LilvNode*
lilv_plugin_get_project(const LilvPlugin* plugin)
{
    lilv_plugin_load_if_necessary(plugin);

    SordNode* lv2_project = sord_new_uri(plugin->world->world,
        (const uint8_t*)LV2_CORE__project);

    SordIter* projects = lilv_world_query_internal(plugin->world,
        plugin->plugin_uri->node,
        lv2_project,
        NULL);

    sord_node_free(plugin->world->world, lv2_project);

    if (sord_iter_end(projects)) {
        sord_iter_free(projects);
        return NULL;
    }

    const SordNode* project = sord_iter_get_node(projects, SORD_OBJECT);

    sord_iter_free(projects);
    return lilv_node_new_from_node(plugin->world, project);
}

static const SordNode*
lilv_plugin_get_author(const LilvPlugin* plugin)
{
    lilv_plugin_load_if_necessary(plugin);

    SordNode* doap_maintainer = sord_new_uri(
        plugin->world->world, NS_DOAP "maintainer");

    SordIter* maintainers = lilv_world_query_internal(
        plugin->world,
        plugin->plugin_uri->node,
        doap_maintainer,
        NULL);

    if (sord_iter_end(maintainers)) {
        sord_iter_free(maintainers);

        LilvNode* project = lilv_plugin_get_project(plugin);
        if (!project) {
            sord_node_free(plugin->world->world, doap_maintainer);
            return NULL;
        }

        maintainers = lilv_world_query_internal(
            plugin->world,
            project->node,
            doap_maintainer,
            NULL);

        lilv_node_free(project);
    }

    sord_node_free(plugin->world->world, doap_maintainer);

    if (sord_iter_end(maintainers)) {
        sord_iter_free(maintainers);
        return NULL;
    }

    const SordNode* author = sord_iter_get_node(maintainers, SORD_OBJECT);

    sord_iter_free(maintainers);
    return author;
}

static LilvNode*
lilv_plugin_get_author_property(const LilvPlugin* plugin, const uint8_t* uri)
{
    const SordNode* author = lilv_plugin_get_author(plugin);
    if (author) {
        SordWorld* sworld = plugin->world->world;
        SordNode* pred = sord_new_uri(sworld, uri);
        LilvNode* ret = lilv_plugin_get_one(plugin, author, pred);
        sord_node_free(sworld, pred);
        return ret;
    }
    return NULL;
}

LilvNode*
lilv_plugin_get_author_name(const LilvPlugin* plugin)
{
    return lilv_plugin_get_author_property(plugin, NS_FOAF "name");
}

LilvNode*
lilv_plugin_get_author_email(const LilvPlugin* plugin)
{
    return lilv_plugin_get_author_property(plugin, NS_FOAF "mbox");
}

LilvNode*
lilv_plugin_get_author_homepage(const LilvPlugin* plugin)
{
    return lilv_plugin_get_author_property(plugin, NS_FOAF "homepage");
}

bool
lilv_plugin_is_replaced(const LilvPlugin* plugin)
{
    return plugin->replaced;
}

LilvUIs*
lilv_plugin_get_uis(const LilvPlugin* plugin)
{
    lilv_plugin_load_if_necessary(plugin);

    SordNode* ui_ui_node = sord_new_uri(plugin->world->world,
        (const uint8_t*)LV2_UI__ui);
    SordNode* ui_binary_node = sord_new_uri(plugin->world->world,
        (const uint8_t*)LV2_UI__binary);

    LilvUIs* result = lilv_uis_new();
    SordIter* uis = lilv_world_query_internal(plugin->world,
        plugin->plugin_uri->node,
        ui_ui_node,
        NULL);

    FOREACH_MATCH(uis) {
        const SordNode* ui = sord_iter_get_node(uis, SORD_OBJECT);

        LilvNode* type = lilv_plugin_get_unique(plugin, ui, plugin->world->uris.rdf_a);
        LilvNode* binary = lilv_plugin_get_one(plugin, ui, plugin->world->uris.lv2_binary);
        if (!binary) {
            binary = lilv_plugin_get_unique(plugin, ui, ui_binary_node);
        }

        if (sord_node_get_type(ui) != SORD_URI
            || !lilv_node_is_uri(type)
            || !lilv_node_is_uri(binary)) {
            lilv_node_free(binary);
            lilv_node_free(type);
            LILV_ERRORF("Corrupt UI <%s>\n", sord_node_get_string(ui));
            continue;
        }

        LilvUI* lilv_ui = lilv_ui_new(
            plugin->world,
            lilv_node_new_from_node(plugin->world, ui),
            type,
            binary);

        zix_tree_insert((ZixTree*)result, lilv_ui, NULL);
    }
    sord_iter_free(uis);

    sord_node_free(plugin->world->world, ui_binary_node);
    sord_node_free(plugin->world->world, ui_ui_node);

    if (lilv_uis_size(result) > 0) {
        return result;
    }
    else {
        lilv_uis_free(result);
        return NULL;
    }
}

LilvNodes*
lilv_plugin_get_related(const LilvPlugin* plugin, const LilvNode* type)
{
    lilv_plugin_load_if_necessary(plugin);

    LilvWorld* const world = plugin->world;
    LilvNodes* const related = lilv_world_find_nodes_internal(
        world,
        NULL,
        world->uris.lv2_appliesTo,
        lilv_plugin_get_uri(plugin)->node);

    if (!type) {
        return related;
    }

    LilvNodes* matches = lilv_nodes_new();
    LILV_FOREACH(nodes, i, related) {
        LilvNode* node = (LilvNode*)lilv_collection_get((ZixTree*)related, i);
        if (lilv_world_ask_internal(
            world, node->node, world->uris.rdf_a, type->node)) {
            zix_tree_insert((ZixTree*)matches,
                lilv_node_new_from_node(world, node->node),
                NULL);
        }
    }

    lilv_nodes_free(related);
    return matches;
}

static SerdEnv*
new_lv2_env(const SerdNode* base)
{
    SerdEnv* env = serd_env_new(base);

#define USTR(s) ((const uint8_t*)(s))
    serd_env_set_prefix_from_strings(env, USTR("doap"), USTR(LILV_NS_DOAP));
    serd_env_set_prefix_from_strings(env, USTR("foaf"), USTR(LILV_NS_FOAF));
    serd_env_set_prefix_from_strings(env, USTR("lv2"), USTR(LILV_NS_LV2));
    serd_env_set_prefix_from_strings(env, USTR("owl"), USTR(LILV_NS_OWL));
    serd_env_set_prefix_from_strings(env, USTR("rdf"), USTR(LILV_NS_RDF));
    serd_env_set_prefix_from_strings(env, USTR("rdfs"), USTR(LILV_NS_RDFS));
    serd_env_set_prefix_from_strings(env, USTR("xsd"), USTR(LILV_NS_XSD));

    return env;
}

static void
maybe_write_prefixes(SerdWriter* writer, SerdEnv* env, FILE* file)
{
    fseek(file, 0, SEEK_END);
    if (ftell(file) == 0) {
        serd_env_foreach(
            env, (SerdPrefixSink)serd_writer_set_prefix, writer);
    }
    else {
        fprintf(file, "\n");
    }
}

void
lilv_plugin_write_description(LilvWorld* world,
    const LilvPlugin* plugin,
    const LilvNode* base_uri,
    FILE* plugin_file)
{
    const LilvNode* subject = lilv_plugin_get_uri(plugin);
    const uint32_t  num_ports = lilv_plugin_get_num_ports(plugin);
    const SerdNode* base = sord_node_to_serd_node(base_uri->node);
    SerdEnv* env = new_lv2_env(base);

    SerdWriter* writer = serd_writer_new(
        SERD_TURTLE,
        (SerdStyle)(SERD_STYLE_ABBREVIATED | SERD_STYLE_CURIED),
        env,
        NULL,
        serd_file_sink,
        plugin_file);

    // Write prefixes if this is a new file
    maybe_write_prefixes(writer, env, plugin_file);

    // Write plugin description
    SordIter* plug_iter = lilv_world_query_internal(
        world, subject->node, NULL, NULL);
    sord_write_iter(plug_iter, writer);

    // Write port descriptions
    for (uint32_t i = 0; i < num_ports; ++i) {
        const LilvPort* port = plugin->ports[i];
        SordIter* port_iter = lilv_world_query_internal(
            world, port->node->node, NULL, NULL);
        sord_write_iter(port_iter, writer);
    }

    serd_writer_free(writer);
    serd_env_free(env);
}

void
lilv_plugin_write_manifest_entry(LilvWorld* world,
    const LilvPlugin* plugin,
    const LilvNode* base_uri,
    FILE* manifest_file,
    const char* plugin_file_path)
{
    const LilvNode* subject = lilv_plugin_get_uri(plugin);
    const SerdNode* base = sord_node_to_serd_node(base_uri->node);
    SerdEnv* env = new_lv2_env(base);

    SerdWriter* writer = serd_writer_new(
        SERD_TURTLE,
        (SerdStyle)(SERD_STYLE_ABBREVIATED | SERD_STYLE_CURIED),
        env,
        NULL,
        serd_file_sink,
        manifest_file);

    // Write prefixes if this is a new file
    maybe_write_prefixes(writer, env, manifest_file);

    // Write manifest entry
    serd_writer_write_statement(
        writer, 0, NULL,
        sord_node_to_serd_node(subject->node),
        sord_node_to_serd_node(plugin->world->uris.rdf_a),
        sord_node_to_serd_node(plugin->world->uris.lv2_Plugin), 0, 0);

    const SerdNode file_node = serd_node_from_string(
        SERD_URI, (const uint8_t*)plugin_file_path);
    serd_writer_write_statement(
        writer, 0, NULL,
        sord_node_to_serd_node(subject->node),
        sord_node_to_serd_node(plugin->world->uris.rdfs_seeAlso),
        &file_node, 0, 0);

    serd_writer_free(writer);
    serd_env_free(env);
}


LilvPluginClass*
lilv_plugin_class_new(LilvWorld* world,
    const SordNode* parent_node,
    const SordNode* uri,
    const char* label)
{
    LilvPluginClass* pc = (LilvPluginClass*)malloc(sizeof(LilvPluginClass));
    pc->world = world;
    pc->uri = lilv_node_new_from_node(world, uri);
    pc->label = lilv_node_new(world, LILV_VALUE_STRING, label);
    pc->parent_uri = (parent_node
        ? lilv_node_new_from_node(world, parent_node)
        : NULL);
    return pc;
}

void
lilv_plugin_class_free(LilvPluginClass* plugin_class)
{
    if (!plugin_class) {
        return;
    }

    lilv_node_free(plugin_class->uri);
    lilv_node_free(plugin_class->parent_uri);
    lilv_node_free(plugin_class->label);
    free(plugin_class);
}

const LilvNode*
lilv_plugin_class_get_parent_uri(const LilvPluginClass* plugin_class)
{
    return plugin_class->parent_uri ? plugin_class->parent_uri : NULL;
}

const LilvNode*
lilv_plugin_class_get_uri(const LilvPluginClass* plugin_class)
{
    return plugin_class->uri;
}

const LilvNode*
lilv_plugin_class_get_label(const LilvPluginClass* plugin_class)
{
    return plugin_class->label;
}

LilvPluginClasses*
lilv_plugin_class_get_children(const LilvPluginClass* plugin_class)
{
    // Returned list doesn't own categories
    LilvPluginClasses* all = plugin_class->world->plugin_classes;
    LilvPluginClasses* result = zix_tree_new(false, lilv_ptr_cmp, NULL, NULL);

    for (ZixTreeIter* i = zix_tree_begin((ZixTree*)all);
        i != zix_tree_end((ZixTree*)all);
        i = zix_tree_iter_next(i)) {
        const LilvPluginClass* c = (LilvPluginClass*)zix_tree_get(i);
        const LilvNode* parent = lilv_plugin_class_get_parent_uri(c);
        if (parent && lilv_node_equals(lilv_plugin_class_get_uri(plugin_class),
            parent)) {
            zix_tree_insert((ZixTree*)result, (LilvPluginClass*)c, NULL);
        }
    }

    return result;
}


LilvPort*
lilv_port_new(LilvWorld* world,
    const SordNode* node,
    uint32_t        index,
    const char* symbol)
{
    LilvPort* port = (LilvPort*)malloc(sizeof(LilvPort));
    port->node = lilv_node_new_from_node(world, node);
    port->index = index;
    port->symbol = lilv_node_new(world, LILV_VALUE_STRING, symbol);
    port->classes = lilv_nodes_new();
    return port;
}

void
lilv_port_free(const LilvPlugin* plugin, LilvPort* port)
{
    if (port) {
        lilv_node_free(port->node);
        lilv_nodes_free(port->classes);
        lilv_node_free(port->symbol);
        free(port);
    }
}

bool
lilv_port_is_a(const LilvPlugin* plugin,
    const LilvPort* port,
    const LilvNode* port_class)
{
    LILV_FOREACH(nodes, i, port->classes) {
        if (lilv_node_equals(lilv_nodes_get(port->classes, i), port_class)) {
            return true;
        }
    }

    return false;
}

bool
lilv_port_has_property(const LilvPlugin* plugin,
    const LilvPort* port,
    const LilvNode* property)
{
    return lilv_world_ask_internal(plugin->world,
        port->node->node,
        plugin->world->uris.lv2_portProperty,
        property->node);
}

bool
lilv_port_supports_event(const LilvPlugin* plugin,
    const LilvPort* port,
    const LilvNode* event_type)
{
    const uint8_t* predicates[] = { (const uint8_t*)LV2_EVENT__supportsEvent,
                                    (const uint8_t*)LV2_ATOM__supports,
                                    NULL };

    for (const uint8_t** pred = predicates; *pred; ++pred) {
        if (lilv_world_ask_internal(plugin->world,
            port->node->node,
            sord_new_uri(plugin->world->world, *pred),
            event_type->node)) {
            return true;
        }
    }
    return false;
}

static LilvNodes*
lilv_port_get_value_by_node(const LilvPlugin* plugin,
    const LilvPort* port,
    const SordNode* predicate)
{
    return lilv_world_find_nodes_internal(plugin->world,
        port->node->node,
        predicate,
        NULL);
}

const LilvNode*
lilv_port_get_node(const LilvPlugin* plugin,
    const LilvPort* port)
{
    return port->node;
}

LilvNodes*
lilv_port_get_value(const LilvPlugin* plugin,
    const LilvPort* port,
    const LilvNode* predicate)
{
    if (!lilv_node_is_uri(predicate)) {
        LILV_ERRORF("Predicate `%s' is not a URI\n",
            sord_node_get_string(predicate->node));
        return NULL;
    }

    return lilv_port_get_value_by_node(plugin, port, predicate->node);
}

LilvNode*
lilv_port_get(const LilvPlugin* plugin,
    const LilvPort* port,
    const LilvNode* predicate)
{
    LilvNodes* values = lilv_port_get_value(plugin, port, predicate);

    LilvNode* value = lilv_node_duplicate(
        values ? lilv_nodes_get_first(values) : NULL);

    lilv_nodes_free(values);
    return value;
}

uint32_t
lilv_port_get_index(const LilvPlugin* plugin,
    const LilvPort* port)
{
    return port->index;
}

const LilvNode*
lilv_port_get_symbol(const LilvPlugin* plugin,
    const LilvPort* port)
{
    return port->symbol;
}

LilvNode*
lilv_port_get_name(const LilvPlugin* plugin,
    const LilvPort* port)
{
    LilvNodes* results = lilv_port_get_value_by_node(
        plugin, port, plugin->world->uris.lv2_name);

    LilvNode* ret = NULL;
    if (results) {
        LilvNode* val = lilv_nodes_get_first(results);
        if (lilv_node_is_string(val)) {
            ret = lilv_node_duplicate(val);
        }
        lilv_nodes_free(results);
    }

    if (!ret) {
        LILV_WARNF("Plugin <%s> port has no (mandatory) doap:name\n",
            lilv_node_as_string(lilv_plugin_get_uri(plugin)));
    }

    return ret;
}

const LilvNodes*
lilv_port_get_classes(const LilvPlugin* plugin,
    const LilvPort* port)
{
    return port->classes;
}

void
lilv_port_get_range(const LilvPlugin* plugin,
    const LilvPort* port,
    LilvNode** def,
    LilvNode** min,
    LilvNode** max)
{
    if (def) {
        LilvNodes* defaults = lilv_port_get_value_by_node(
            plugin, port, plugin->world->uris.lv2_default);
        *def = defaults
            ? lilv_node_duplicate(lilv_nodes_get_first(defaults))
            : NULL;
        lilv_nodes_free(defaults);
    }
    if (min) {
        LilvNodes* minimums = lilv_port_get_value_by_node(
            plugin, port, plugin->world->uris.lv2_minimum);
        *min = minimums
            ? lilv_node_duplicate(lilv_nodes_get_first(minimums))
            : NULL;
        lilv_nodes_free(minimums);
    }
    if (max) {
        LilvNodes* maximums = lilv_port_get_value_by_node(
            plugin, port, plugin->world->uris.lv2_maximum);
        *max = maximums
            ? lilv_node_duplicate(lilv_nodes_get_first(maximums))
            : NULL;
        lilv_nodes_free(maximums);
    }
}

LilvScalePoints*
lilv_port_get_scale_points(const LilvPlugin* plugin,
    const LilvPort* port)
{
    SordIter* points = lilv_world_query_internal(
        plugin->world,
        port->node->node,
        sord_new_uri(plugin->world->world, (const uint8_t*)LV2_CORE__scalePoint),
        NULL);

    LilvScalePoints* ret = NULL;
    if (!sord_iter_end(points)) {
        ret = lilv_scale_points_new();
    }

    FOREACH_MATCH(points) {
        const SordNode* point = sord_iter_get_node(points, SORD_OBJECT);

        LilvNode* value = lilv_plugin_get_unique(plugin,
            point,
            plugin->world->uris.rdf_value);

        LilvNode* label = lilv_plugin_get_unique(plugin,
            point,
            plugin->world->uris.rdfs_label);

        if (value && label) {
            zix_tree_insert(
                (ZixTree*)ret, lilv_scale_point_new(value, label), NULL);
        }
    }
    sord_iter_free(points);

    assert(!ret || lilv_nodes_size(ret) > 0);
    return ret;
}

LilvNodes*
lilv_port_get_properties(const LilvPlugin* plugin,
    const LilvPort* port)
{
    LilvNode* pred = lilv_node_new_from_node(
        plugin->world, plugin->world->uris.lv2_portProperty);
    LilvNodes* ret = lilv_port_get_value(plugin, port, pred);
    lilv_node_free(pred);
    return ret;
}


typedef enum {
    LILV_LANG_MATCH_NONE,     ///< Language does not match at all
    LILV_LANG_MATCH_PARTIAL,  ///< Partial (language, but not country) match
    LILV_LANG_MATCH_EXACT     ///< Exact (language and country) match
} LilvLangMatch;

static LilvLangMatch
lilv_lang_matches(const char* a, const char* b)
{
    if (!a || !b) {
        return LILV_LANG_MATCH_NONE;
    }
    else if (!strcmp(a, b)) {
        return LILV_LANG_MATCH_EXACT;
    }

    const char* a_dash = strchr(a, '-');
    const size_t a_lang_len = a_dash ? (size_t)(a_dash - a) : strlen(a);
    const char* b_dash = strchr(b, '-');
    const size_t b_lang_len = b_dash ? (size_t)(b_dash - b) : strlen(b);

    if (a_lang_len == b_lang_len && !strncmp(a, b, a_lang_len)) {
        return LILV_LANG_MATCH_PARTIAL;
    }

    return LILV_LANG_MATCH_NONE;
}

static LilvNodes*
lilv_nodes_from_stream_objects_i18n(LilvWorld* world,
    SordIter* stream,
    SordQuadIndex field)
{
    LilvNodes* values = lilv_nodes_new();
    const SordNode* nolang = NULL;  // Untranslated value
    const SordNode* partial = NULL;  // Partial language match
    char* syslang = lilv_get_lang();
    FOREACH_MATCH(stream) {
        const SordNode* value = sord_iter_get_node(stream, field);
        if (sord_node_get_type(value) == SORD_LITERAL) {
            const char* lang = sord_node_get_language(value);

            if (!lang) {
                nolang = value;
            }
            else {
                switch (lilv_lang_matches(lang, syslang)) {
                case LILV_LANG_MATCH_EXACT:
                    // Exact language match, add to results
                    zix_tree_insert((ZixTree*)values,
                        lilv_node_new_from_node(world, value),
                        NULL);
                    break;
                case LILV_LANG_MATCH_PARTIAL:
                    // Partial language match, save in case we find no exact
                    partial = value;
                    break;
                case LILV_LANG_MATCH_NONE:
                    break;
                }
            }
        }
        else {
            zix_tree_insert((ZixTree*)values,
                lilv_node_new_from_node(world, value),
                NULL);
        }
    }
    sord_iter_free(stream);
    free(syslang);

    if (lilv_nodes_size(values) > 0) {
        return values;
    }

    const SordNode* best = nolang;
    if (syslang && partial) {
        // Partial language match for system language
        best = partial;
    }
    else if (!best) {
        // No languages matches at all, and no untranslated value
        // Use any value, if possible
        best = partial;
    }

    if (best) {
        zix_tree_insert(
            (ZixTree*)values, lilv_node_new_from_node(world, best), NULL);
    }
    else {
        // No matches whatsoever
        lilv_nodes_free(values);
        values = NULL;
    }

    return values;
}

LilvNodes*
lilv_nodes_from_stream_objects(LilvWorld* world,
    SordIter* stream,
    SordQuadIndex field)
{
    if (sord_iter_end(stream)) {
        sord_iter_free(stream);
        return NULL;
    }
    else if (world->opt.filter_language) {
        return lilv_nodes_from_stream_objects_i18n(world, stream, field);
    }
    else {
        LilvNodes* values = lilv_nodes_new();
        FOREACH_MATCH(stream) {
            const SordNode* value = sord_iter_get_node(stream, field);
            LilvNode* node = lilv_node_new_from_node(world, value);
            if (node) {
                zix_tree_insert((ZixTree*)values, node, NULL);
            }
        }
        sord_iter_free(stream);
        return values;
    }
}


/** Ownership of value and label is taken */
LilvScalePoint*
lilv_scale_point_new(LilvNode* value, LilvNode* label)
{
    LilvScalePoint* point = (LilvScalePoint*)malloc(sizeof(LilvScalePoint));
    point->value = value;
    point->label = label;
    return point;
}

void
lilv_scale_point_free(LilvScalePoint* point)
{
    if (point) {
        lilv_node_free(point->value);
        lilv_node_free(point->label);
        free(point);
    }
}

const LilvNode*
lilv_scale_point_get_value(const LilvScalePoint* point)
{
    return point->value;
}

const LilvNode*
lilv_scale_point_get_label(const LilvScalePoint* point)
{
    return point->label;
}


#define USTR(s) ((const uint8_t*)(s))

typedef struct {
    void* value;  ///< Value/Object
    size_t   size;   ///< Size of value
    uint32_t key;    ///< Key/Predicate (URID)
    uint32_t type;   ///< Type of value (URID)
    uint32_t flags;  ///< State flags (POD, etc)
} Property;

typedef struct {
    char* symbol; ///< Symbol of port
    LV2_Atom* atom;   ///< Value in port
} PortValue;

typedef struct {
    char* abs;  ///< Absolute path of actual file
    char* rel;  ///< Abstract path (relative path in state dir)
} PathMap;

typedef struct {
    size_t    n;
    Property* props;
} PropertyArray;

struct LilvStateImpl {
    LilvNode* plugin_uri;   ///< Plugin URI
    LilvNode* uri;          ///< State/preset URI
    char* dir;          ///< Save directory (if saved)
    char* scratch_dir;  ///< Directory for files created by plugin
    char* copy_dir;     ///< Directory for snapshots of external files
    char* link_dir;     ///< Directory for links to external files
    char* label;        ///< State/Preset label
    ZixTree* abs2rel;      ///< PathMap sorted by abs
    ZixTree* rel2abs;      ///< PathMap sorted by rel
    PropertyArray props;        ///< State properties
    PropertyArray metadata;     ///< State metadata
    PortValue* values;       ///< Port values
    uint32_t      atom_Path;    ///< atom:Path URID
    uint32_t      n_values;     ///< Number of port values
};

static int
abs_cmp(const void* a, const void* b, void* user_data)
{
    return strcmp(((const PathMap*)a)->abs, ((const PathMap*)b)->abs);
}

static int
rel_cmp(const void* a, const void* b, void* user_data)
{
    return strcmp(((const PathMap*)a)->rel, ((const PathMap*)b)->rel);
}

static int
property_cmp(const void* a, const void* b)
{
    return ((const Property*)a)->key - ((const Property*)b)->key;
}

static int
value_cmp(const void* a, const void* b)
{
    return strcmp(((const PortValue*)a)->symbol,
        ((const PortValue*)b)->symbol);
}

static void
path_rel_free(void* ptr)
{
    free(((PathMap*)ptr)->abs);
    free(((PathMap*)ptr)->rel);
    free(ptr);
}

static PortValue*
append_port_value(LilvState* state,
    const char* port_symbol,
    const void* value,
    uint32_t    size,
    uint32_t    type)
{
    PortValue* pv = NULL;
    if (value) {
        state->values = (PortValue*)realloc(
            state->values, (++state->n_values) * sizeof(PortValue));

        pv = &state->values[state->n_values - 1];
        pv->symbol = lilv_strdup(port_symbol);
        pv->atom = (LV2_Atom*)malloc(sizeof(LV2_Atom) + size);
        pv->atom->size = size;
        pv->atom->type = type;
        memcpy(pv->atom + 1, value, size);
    }
    return pv;
}

static const char*
lilv_state_rel2abs(const LilvState* state, const char* path)
{
    ZixTreeIter* iter = NULL;
    const PathMap key = { NULL, (char*)path };
    if (state->rel2abs && !zix_tree_find(state->rel2abs, &key, &iter)) {
        return ((const PathMap*)zix_tree_get(iter))->abs;
    }
    return path;
}

static void
append_property(LilvState* state,
    PropertyArray* array,
    uint32_t       key,
    const void* value,
    size_t         size,
    uint32_t       type,
    uint32_t       flags)
{
    array->props = (Property*)realloc(
        array->props, (++array->n) * sizeof(Property));

    Property* const prop = &array->props[array->n - 1];
    if ((flags & LV2_STATE_IS_POD) || type == state->atom_Path) {
        prop->value = malloc(size);
        memcpy(prop->value, value, size);
    }
    else {
        prop->value = (void*)value;
    }

    prop->size = size;
    prop->key = key;
    prop->type = type;
    prop->flags = flags;
}

static const Property*
find_property(const LilvState* const state, const uint32_t key)
{
    const Property search_key = { NULL, 0, key, 0, 0 };

    return (const Property*)bsearch(&search_key,
        state->props.props,
        state->props.n,
        sizeof(Property),
        property_cmp);
}

static LV2_State_Status
store_callback(LV2_State_Handle handle,
    uint32_t         key,
    const void* value,
    size_t           size,
    uint32_t         type,
    uint32_t         flags)
{
    LilvState* const state = (LilvState*)handle;

    if (!key) {
        return LV2_STATE_ERR_UNKNOWN; // TODO: Add status for bad arguments
    }

    if (find_property((const LilvState*)handle, key)) {
        return LV2_STATE_ERR_UNKNOWN; // TODO: Add status for duplicate keys
    }

    append_property(state, &state->props, key, value, size, type, flags);
    return LV2_STATE_SUCCESS;
}

static const void*
retrieve_callback(LV2_State_Handle handle,
    uint32_t         key,
    size_t* size,
    uint32_t* type,
    uint32_t* flags)
{
    const Property* const prop = find_property((const LilvState*)handle, key);

    if (prop) {
        *size = prop->size;
        *type = prop->type;
        *flags = prop->flags;
        return prop->value;
    }
    return NULL;
}

static bool
lilv_state_has_path(const char* path, const void* state)
{
    return lilv_state_rel2abs((const LilvState*)state, path) != path;
}

static char*
make_path(LV2_State_Make_Path_Handle handle, const char* path)
{
    LilvState* state = (LilvState*)handle;
    lilv_mkdir_p(state->dir);

    return lilv_path_join(state->dir, path);
}

static char*
abstract_path(LV2_State_Map_Path_Handle handle,
    const char* abs_path)
{
    LilvState* state = (LilvState*)handle;
    char* path = NULL;
    char* real_path = lilv_realpath(abs_path);
    const PathMap key = { real_path, NULL };
    ZixTreeIter* iter = NULL;

    if (abs_path[0] == '\0') {
        return lilv_strdup(abs_path);
    }
    else if (!zix_tree_find(state->abs2rel, &key, &iter)) {
        // Already mapped path in a previous call
        PathMap* pm = (PathMap*)zix_tree_get(iter);
        free(real_path);
        return lilv_strdup(pm->rel);
    }
    else if (lilv_path_is_child(real_path, state->dir)) {
        // File in state directory (loaded, or created by plugin during save)
        path = lilv_path_relative_to(real_path, state->dir);
    }
    else if (lilv_path_is_child(real_path, state->scratch_dir)) {
        // File created by plugin earlier
        path = lilv_path_relative_to(real_path, state->scratch_dir);
        if (state->copy_dir) {
            int st = lilv_mkdir_p(state->copy_dir);
            if (st) {
                LILV_ERRORF("Error creating directory %s (%s)\n",
                    state->copy_dir, strerror(st));
            }

            char* cpath = lilv_path_join(state->copy_dir, path);
            char* copy = lilv_get_latest_copy(real_path, cpath);
            if (!copy || !lilv_file_equals(real_path, copy)) {
                // No recent enough copy, make a new one
                free(copy);
                copy = lilv_find_free_path(cpath, lilv_path_exists, NULL);
                if ((st = lilv_copy_file(real_path, copy))) {
                    LILV_ERRORF("Error copying state file %s (%s)\n",
                        copy, strerror(st));
                }
            }
            free(real_path);
            free(cpath);

            // Refer to the latest copy in plugin state
            real_path = copy;
        }
    }
    else if (state->link_dir) {
        // New path outside state directory, make a link
        const char* slash = strrchr(real_path, '/');
        const char* name = slash ? (slash + 1) : real_path;

        // Find a free name in the (virtual) state directory
        path = lilv_find_free_path(name, lilv_state_has_path, state);
    }
    else {
        // No link directory, preserve absolute path
        path = lilv_strdup(abs_path);
    }

    // Add record to path mapping
    PathMap* pm = (PathMap*)malloc(sizeof(PathMap));
    pm->abs = real_path;
    pm->rel = lilv_strdup(path);
    zix_tree_insert(state->abs2rel, pm, NULL);
    zix_tree_insert(state->rel2abs, pm, NULL);

    return path;
}

static char*
absolute_path(LV2_State_Map_Path_Handle handle,
    const char* state_path)
{
    LilvState* state = (LilvState*)handle;
    char* path = NULL;
    if (lilv_path_is_absolute(state_path)) {
        // Absolute path, return identical path
        path = lilv_strdup(state_path);
    }
    else if (state->dir) {
        // Relative path inside state directory
        path = lilv_path_join(state->dir, state_path);
    }
    else {
        // State has not been saved, unmap
        path = lilv_strdup(lilv_state_rel2abs(state, state_path));
    }

    return path;
}

/** Return a new features array with built-in features added to `features`. */
static const LV2_Feature**
add_features(const LV2_Feature* const* features,
    const LV2_Feature* map,
    const LV2_Feature* make,
    const LV2_Feature* free)
{
    size_t n_features = 0;
    for (; features && features[n_features]; ++n_features) {}

    const LV2_Feature** ret = (const LV2_Feature**)calloc(
        n_features + 4, sizeof(LV2_Feature*));

    if (features) {
        memcpy(ret, features, n_features * sizeof(LV2_Feature*));
    }

    size_t i = n_features;
    if (map) {
        ret[i++] = map;
    }
    if (make) {
        ret[i++] = make;
    }
    if (free) {
        ret[i++] = free;
    }

    return ret;
}

static char*
absolute_dir(const char* path)
{
    char* abs_path = lilv_path_absolute(path);
    char* base = lilv_path_join(abs_path, NULL);
    free(abs_path);
    return base;
}

static const char*
state_strerror(LV2_State_Status st)
{
    switch (st) {
    case LV2_STATE_SUCCESS:         return "Completed successfully";
    case LV2_STATE_ERR_BAD_TYPE:    return "Unsupported type";
    case LV2_STATE_ERR_BAD_FLAGS:   return "Unsupported flags";
    case LV2_STATE_ERR_NO_FEATURE:  return "Missing features";
    case LV2_STATE_ERR_NO_PROPERTY: return "Missing property";
    default:                        return "Unknown error";
    }
}

static void
lilv_free_path(LV2_State_Free_Path_Handle handle, char* path)
{
    lilv_free(path);
}

LilvState*
lilv_state_new_from_instance(const LilvPlugin* plugin,
    LilvInstance* instance,
    LV2_URID_Map* map,
    const char* scratch_dir,
    const char* copy_dir,
    const char* link_dir,
    const char* save_dir,
    LilvGetPortValueFunc       get_value,
    void* user_data,
    uint32_t                   flags,
    const LV2_Feature* const* features)
{
    const LV2_Feature** sfeatures = NULL;
    LilvWorld* const    world = plugin->world;
    LilvState* const    state = (LilvState*)calloc(1, sizeof(LilvState));
    state->plugin_uri = lilv_node_duplicate(lilv_plugin_get_uri(plugin));
    state->abs2rel = zix_tree_new(false, abs_cmp, NULL, path_rel_free);
    state->rel2abs = zix_tree_new(false, rel_cmp, NULL, NULL);
    state->scratch_dir = scratch_dir ? absolute_dir(scratch_dir) : NULL;
    state->copy_dir = copy_dir ? absolute_dir(copy_dir) : NULL;
    state->link_dir = link_dir ? absolute_dir(link_dir) : NULL;
    state->dir = save_dir ? absolute_dir(save_dir) : NULL;
    state->atom_Path = map->map(map->handle, LV2_ATOM__Path);

    LV2_State_Map_Path  pmap = { state, abstract_path, absolute_path };
    LV2_Feature         pmap_feature = { LV2_STATE__mapPath, &pmap };
    LV2_State_Make_Path pmake = { state, make_path };
    LV2_Feature         pmake_feature = { LV2_STATE__makePath, &pmake };
    LV2_State_Free_Path pfree = { NULL, lilv_free_path };
    LV2_Feature         pfree_feature = { LV2_STATE__freePath, &pfree };
    features = sfeatures = add_features(features, &pmap_feature,
        save_dir ? &pmake_feature : NULL,
        &pfree_feature);

    // Store port values
    if (get_value) {
        LilvNode* lv2_ControlPort = lilv_new_uri(world, LILV_URI_CONTROL_PORT);
        LilvNode* lv2_InputPort = lilv_new_uri(world, LILV_URI_INPUT_PORT);
        for (uint32_t i = 0; i < plugin->num_ports; ++i) {
            const LilvPort* const port = plugin->ports[i];
            if (lilv_port_is_a(plugin, port, lv2_ControlPort)
                && lilv_port_is_a(plugin, port, lv2_InputPort)) {
                uint32_t size, type;
                const char* sym = lilv_node_as_string(port->symbol);
                const void* value = get_value(sym, user_data, &size, &type);
                append_port_value(state, sym, value, size, type);
            }
        }
        lilv_node_free(lv2_ControlPort);
        lilv_node_free(lv2_InputPort);
    }

    // Store properties
    const LV2_Descriptor* desc = instance->lv2_descriptor;
    const LV2_State_Interface* iface = (desc->extension_data)
        ? (const LV2_State_Interface*)desc->extension_data(LV2_STATE__interface)
        : NULL;

    if (iface) {
        LV2_State_Status st = iface->save(
            instance->lv2_handle, store_callback, state, flags, features);
        if (st) {
            LILV_ERRORF("Error saving plugin state: %s\n", state_strerror(st));
            free(state->props.props);
            state->props.props = NULL;
            state->props.n = 0;
        }
        else {
            qsort(state->props.props, state->props.n, sizeof(Property), property_cmp);
        }
    }

    qsort(state->values, state->n_values, sizeof(PortValue), value_cmp);

    free(sfeatures);
    return state;
}

void
lilv_state_emit_port_values(const LilvState* state,
    LilvSetPortValueFunc set_value,
    void* user_data)
{
    for (uint32_t i = 0; i < state->n_values; ++i) {
        const PortValue* value = &state->values[i];
        const LV2_Atom* atom = value->atom;
        set_value(value->symbol, user_data, atom + 1, atom->size, atom->type);
    }
}

void
lilv_state_restore(const LilvState* state,
    LilvInstance* instance,
    LilvSetPortValueFunc       set_value,
    void* user_data,
    uint32_t                   flags,
    const LV2_Feature* const* features)
{
    if (!state) {
        LILV_ERROR("lilv_state_restore() called on NULL state\n");
        return;
    }

    LV2_State_Map_Path map_path = {
        (LilvState*)state, abstract_path, absolute_path };
    LV2_Feature map_feature = { LV2_STATE__mapPath, &map_path };

    LV2_State_Free_Path free_path = { NULL, lilv_free_path };
    LV2_Feature         free_feature = { LV2_STATE__freePath, &free_path };

    if (instance) {
        const LV2_Descriptor* desc = instance->lv2_descriptor;
        if (desc->extension_data) {
            const LV2_State_Interface* iface = (const LV2_State_Interface*)
                desc->extension_data(LV2_STATE__interface);

            if (iface && iface->restore) {
                const LV2_Feature** sfeatures = add_features(
                    features, &map_feature, NULL, &free_feature);

                iface->restore(instance->lv2_handle, retrieve_callback,
                    (LV2_State_Handle)state, flags, sfeatures);

                free(sfeatures);
            }
        }
    }


    if (set_value) {
        lilv_state_emit_port_values(state, set_value, user_data);
    }
}

static void
set_state_dir_from_model(LilvState* state, const SordNode* graph)
{
    if (!state->dir && graph) {
        const char* uri = (const char*)sord_node_get_string(graph);
        char* path = lilv_file_uri_parse(uri, NULL);

        state->dir = lilv_dir_path(path);
        free(path);
    }
    assert(!state->dir || lilv_path_is_absolute(state->dir));
}

static LilvState*
new_state_from_model(LilvWorld* world,
    LV2_URID_Map* map,
    SordModel* model,
    const SordNode* node,
    const char* dir)
{
    // Check that we know at least something about this state subject
    if (!sord_ask(model, node, 0, 0, 0)) {
        return NULL;
    }

    // Allocate state
    LilvState* const state = (LilvState*)calloc(1, sizeof(LilvState));
    state->dir = lilv_dir_path(dir);
    state->atom_Path = map->map(map->handle, LV2_ATOM__Path);
    state->uri = lilv_node_new_from_node(world, node);

    // Get the plugin URI this state applies to
    SordIter* i = sord_search(model, node, world->uris.lv2_appliesTo, 0, 0);
    if (i) {
        const SordNode* object = sord_iter_get_node(i, SORD_OBJECT);
        const SordNode* graph = sord_iter_get_node(i, SORD_GRAPH);
        state->plugin_uri = lilv_node_new_from_node(world, object);
        set_state_dir_from_model(state, graph);
        sord_iter_free(i);
    }
    else if (sord_ask(model,
        node,
        world->uris.rdf_a,
        world->uris.lv2_Plugin, 0)) {
        // Loading plugin description as state (default state)
        state->plugin_uri = lilv_node_new_from_node(world, node);
    }
    else {
        LILV_ERRORF("State %s missing lv2:appliesTo property\n",
            sord_node_get_string(node));
    }

    // Get the state label
    i = sord_search(model, node, world->uris.rdfs_label, NULL, NULL);
    if (i) {
        const SordNode* object = sord_iter_get_node(i, SORD_OBJECT);
        const SordNode* graph = sord_iter_get_node(i, SORD_GRAPH);
        state->label = lilv_strdup((const char*)sord_node_get_string(object));
        set_state_dir_from_model(state, graph);
        sord_iter_free(i);
    }

    Sratom* sratom = sratom_new(map);
    SerdChunk      chunk = { NULL, 0 };
    LV2_Atom_Forge forge;
    lv2_atom_forge_init(&forge, map);
    lv2_atom_forge_set_sink(
        &forge, sratom_forge_sink, sratom_forge_deref, &chunk);

    // Get port values
    SordIter* ports = sord_search(model, node, world->uris.lv2_port, 0, 0);
    FOREACH_MATCH(ports) {
        const SordNode* port = sord_iter_get_node(ports, SORD_OBJECT);

        SordNode* label = sord_get(model, port, world->uris.rdfs_label, 0, 0);
        SordNode* symbol = sord_get(model, port, world->uris.lv2_symbol, 0, 0);
        SordNode* value = sord_get(model, port, world->uris.pset_value, 0, 0);
        if (!value) {
            value = sord_get(model, port, world->uris.lv2_default, 0, 0);
        }
        if (!symbol) {
            LILV_ERRORF("State `%s' port missing symbol.\n",
                sord_node_get_string(node));
        }
        else if (value) {
            chunk.len = 0;
            sratom_read(sratom, &forge, world->world, model, value);
            const LV2_Atom* atom = (const LV2_Atom*)chunk.buf;

            append_port_value(state,
                (const char*)sord_node_get_string(symbol),
                LV2_ATOM_BODY_CONST(atom),
                atom->size, atom->type);

            if (label) {
                lilv_state_set_label(state,
                    (const char*)sord_node_get_string(label));
            }
        }
        sord_node_free(world->world, value);
        sord_node_free(world->world, symbol);
        sord_node_free(world->world, label);
    }
    sord_iter_free(ports);

    // Get properties
    SordNode* statep = sord_new_uri(world->world, USTR(LV2_STATE__state));
    SordNode* state_node = sord_get(model, node, statep, NULL, NULL);
    if (state_node) {
        SordIter* props = sord_search(model, state_node, 0, 0, 0);
        FOREACH_MATCH(props) {
            const SordNode* p = sord_iter_get_node(props, SORD_PREDICATE);
            const SordNode* o = sord_iter_get_node(props, SORD_OBJECT);
            const char* key = (const char*)sord_node_get_string(p);

            chunk.len = 0;
            lv2_atom_forge_set_sink(
                &forge, sratom_forge_sink, sratom_forge_deref, &chunk);

            sratom_read(sratom, &forge, world->world, model, o);
            const LV2_Atom* atom = (const LV2_Atom*)chunk.buf;
            uint32_t        flags = LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE;
            Property        prop = { NULL, 0, 0, 0, flags };

            prop.key = map->map(map->handle, key);
            prop.type = atom->type;
            prop.size = atom->size;
            prop.value = malloc(atom->size);
            memcpy(prop.value, LV2_ATOM_BODY_CONST(atom), atom->size);
            if (atom->type == forge.Path) {
                prop.flags = LV2_STATE_IS_POD;
            }

            if (prop.value) {
                state->props.props = (Property*)realloc(
                    state->props.props, (++state->props.n) * sizeof(Property));
                state->props.props[state->props.n - 1] = prop;
            }
        }
        sord_iter_free(props);
    }
    sord_node_free(world->world, state_node);
    sord_node_free(world->world, statep);

    serd_free((void*)chunk.buf);
    sratom_free(sratom);

    if (state->props.props) {
        qsort(state->props.props, state->props.n, sizeof(Property), property_cmp);
    }
    if (state->values) {
        qsort(state->values, state->n_values, sizeof(PortValue), value_cmp);
    }

    return state;
}

LilvState*
lilv_state_new_from_world(LilvWorld* world,
    LV2_URID_Map* map,
    const LilvNode* node)
{
    if (!lilv_node_is_uri(node) && !lilv_node_is_blank(node)) {
        LILV_ERRORF("Subject `%s' is not a URI or blank node.\n",
            lilv_node_as_string(node));
        return NULL;
    }

    return new_state_from_model(world, map, world->model, node->node, NULL);
}

LilvState*
lilv_state_new_from_file(LilvWorld* world,
    LV2_URID_Map* map,
    const LilvNode* subject,
    const char* path)
{
    if (subject && !lilv_node_is_uri(subject)
        && !lilv_node_is_blank(subject)) {
        LILV_ERRORF("Subject `%s' is not a URI or blank node.\n",
            lilv_node_as_string(subject));
        return NULL;
    }

    uint8_t* abs_path = (uint8_t*)lilv_path_absolute(path);
    SerdNode    node = serd_node_new_file_uri(abs_path, NULL, NULL, true);
    SerdEnv* env = serd_env_new(&node);
    SordModel* model = sord_new(world->world, SORD_SPO, false);
    SerdReader* reader = sord_new_reader(model, env, SERD_TURTLE, NULL);

    serd_reader_read_file(reader, node.buf);

    SordNode* subject_node = (subject)
        ? subject->node
        : sord_node_from_serd_node(world->world, env, &node, NULL, NULL);

    char* dirname = lilv_dirname(path);
    char* real_path = lilv_realpath(dirname);
    char* dir_path = lilv_dir_path(real_path);
    LilvState* state =
        new_state_from_model(world, map, model, subject_node, dir_path);
    free(dir_path);
    free(real_path);
    free(dirname);

    serd_node_free(&node);
    free(abs_path);
    serd_reader_free(reader);
    sord_free(model);
    serd_env_free(env);
    return state;
}

static void
set_prefixes(SerdEnv* env)
{
#define SET_PSET(e, p, u) serd_env_set_prefix_from_strings(e, p, u)
    SET_PSET(env, USTR("atom"), USTR(LV2_ATOM_PREFIX));
    SET_PSET(env, USTR("lv2"), USTR(LV2_CORE_PREFIX));
    SET_PSET(env, USTR("pset"), USTR(LV2_PRESETS_PREFIX));
    SET_PSET(env, USTR("rdf"), USTR(LILV_NS_RDF));
    SET_PSET(env, USTR("rdfs"), USTR(LILV_NS_RDFS));
    SET_PSET(env, USTR("state"), USTR(LV2_STATE_PREFIX));
    SET_PSET(env, USTR("xsd"), USTR(LILV_NS_XSD));
}

LilvState*
lilv_state_new_from_string(LilvWorld* world,
    LV2_URID_Map* map,
    const char* str)
{
    if (!str) {
        return NULL;
    }

    SerdNode    base = SERD_NODE_NULL;
    SerdEnv* env = serd_env_new(&base);
    SordModel* model = sord_new(world->world, SORD_SPO | SORD_OPS, false);
    SerdReader* reader = sord_new_reader(model, env, SERD_TURTLE, NULL);

    set_prefixes(env);
    serd_reader_read_string(reader, USTR(str));

    SordNode* o = sord_new_uri(world->world, USTR(LV2_PRESETS__Preset));
    SordNode* s = sord_get(model, NULL, world->uris.rdf_a, o, NULL);

    LilvState* state = new_state_from_model(world, map, model, s, NULL);

    sord_node_free(world->world, s);
    sord_node_free(world->world, o);
    serd_reader_free(reader);
    sord_free(model);
    serd_env_free(env);

    return state;
}

static SerdWriter*
ttl_writer(SerdSink sink, void* stream, const SerdNode* base, SerdEnv** new_env)
{
    SerdURI base_uri = SERD_URI_NULL;
    if (base && base->buf) {
        serd_uri_parse(base->buf, &base_uri);
    }

    SerdEnv* env = *new_env ? *new_env : serd_env_new(base);
    set_prefixes(env);

    SerdWriter* writer = serd_writer_new(
        SERD_TURTLE,
        (SerdStyle)(SERD_STYLE_RESOLVED |
            SERD_STYLE_ABBREVIATED |
            SERD_STYLE_CURIED),
        env,
        &base_uri,
        sink,
        stream);

    if (!*new_env) {
        *new_env = env;
    }

    return writer;
}

static SerdWriter*
ttl_file_writer(FILE* fd, const SerdNode* node, SerdEnv** env)
{
    SerdWriter* writer = ttl_writer(serd_file_sink, fd, node, env);

    fseek(fd, 0, SEEK_END);
    if (ftell(fd) == 0) {
        serd_env_foreach(*env, (SerdPrefixSink)serd_writer_set_prefix, writer);
    }
    else {
        fprintf(fd, "\n");
    }

    return writer;
}

static void
add_to_model(SordWorld* world,
    SerdEnv* env,
    SordModel* model,
    const SerdNode s,
    const SerdNode p,
    const SerdNode o)
{
    SordNode* ss = sord_node_from_serd_node(world, env, &s, NULL, NULL);
    SordNode* sp = sord_node_from_serd_node(world, env, &p, NULL, NULL);
    SordNode* so = sord_node_from_serd_node(world, env, &o, NULL, NULL);

    SordQuad quad = { ss, sp, so, NULL };
    sord_add(model, quad);

    sord_node_free(world, ss);
    sord_node_free(world, sp);
    sord_node_free(world, so);
}

static void
remove_manifest_entry(SordWorld* world, SordModel* model, const char* subject)
{
    SordNode* s = sord_new_uri(world, USTR(subject));
    SordIter* i = sord_search(model, s, NULL, NULL, NULL);
    while (!sord_iter_end(i)) {
        sord_erase(model, i);
    }
    sord_iter_free(i);
    sord_node_free(world, s);
}

static int
write_manifest(LilvWorld* world,
    SerdEnv* env,
    SordModel* model,
    const SerdNode* file_uri)
{
    char* const path = (char*)serd_file_uri_parse(file_uri->buf, NULL);
    FILE* const wfd = fopen(path, "w");
    if (!wfd) {
        LILV_ERRORF("Failed to open %s for writing (%s)\n",
            path, strerror(errno));

        serd_free(path);
        return 1;
    }

    SerdWriter* writer = ttl_file_writer(wfd, file_uri, &env);
    sord_write(model, writer, NULL);
    serd_writer_free(writer);
    fclose(wfd);
    serd_free(path);
    return 0;
}

static int
add_state_to_manifest(LilvWorld* lworld,
    const LilvNode* plugin_uri,
    const char* manifest_path,
    const char* state_uri,
    const char* state_path)
{
    SordWorld* world = lworld->world;
    SerdNode    manifest = serd_node_new_file_uri(USTR(manifest_path), 0, 0, 1);
    SerdNode    file = serd_node_new_file_uri(USTR(state_path), 0, 0, 1);
    SerdEnv* env = serd_env_new(&manifest);
    SordModel* model = sord_new(world, SORD_SPO, false);

    FILE* rfd = fopen(manifest_path, "r");
    if (rfd) {
        // Read manifest into model
        SerdReader* reader = sord_new_reader(model, env, SERD_TURTLE, NULL);
        lilv_flock(rfd, true);
        serd_reader_read_file_handle(reader, rfd, manifest.buf);
        serd_reader_free(reader);
    }

    // Choose state URI (use file URI if not given)
    if (!state_uri) {
        state_uri = (const char*)file.buf;
    }

    // Remove any existing manifest entries for this state
    remove_manifest_entry(world, model, state_uri);

    // Add manifest entry for this state to model
    SerdNode s = serd_node_from_string(SERD_URI, USTR(state_uri));

    // <state> a pset:Preset
    add_to_model(world, env, model,
        s,
        serd_node_from_string(SERD_URI, USTR(LILV_NS_RDF "type")),
        serd_node_from_string(SERD_URI, USTR(LV2_PRESETS__Preset)));

    // <state> a pset:Preset
    add_to_model(world, env, model,
        s,
        serd_node_from_string(SERD_URI, USTR(LILV_NS_RDF "type")),
        serd_node_from_string(SERD_URI, USTR(LV2_PRESETS__Preset)));

    // <state> rdfs:seeAlso <file>
    add_to_model(world, env, model,
        s,
        serd_node_from_string(SERD_URI, USTR(LILV_NS_RDFS "seeAlso")),
        file);

    // <state> lv2:appliesTo <plugin>
    add_to_model(world, env, model,
        s,
        serd_node_from_string(SERD_URI, USTR(LV2_CORE__appliesTo)),
        serd_node_from_string(SERD_URI,
            USTR(lilv_node_as_string(plugin_uri))));

    // Write manifest model to file
    write_manifest(lworld, env, model, &manifest);

    sord_free(model);
    serd_node_free(&file);
    serd_node_free(&manifest);
    serd_env_free(env);

    if (rfd) {
        lilv_flock(rfd, false);
        fclose(rfd);
    }

    return 0;
}

static bool
link_exists(const char* path, const void* data)
{
    const char* target = (const char*)data;
    if (!lilv_path_exists(path, NULL)) {
        return false;
    }
    char* real_path = lilv_realpath(path);
    bool  matches = !strcmp(real_path, target);
    free(real_path);
    return !matches;
}

static int
maybe_symlink(const char* oldpath, const char* newpath)
{
    return link_exists(newpath, oldpath) ? 0 : lilv_symlink(oldpath, newpath);
}

static void
write_property_array(const LilvState* state,
    const PropertyArray* array,
    Sratom* sratom,
    uint32_t             flags,
    const SerdNode* subject,
    LV2_URID_Unmap* unmap,
    const char* dir)
{
    for (uint32_t i = 0; i < array->n; ++i) {
        Property* prop = &array->props[i];
        const char* key = unmap->unmap(unmap->handle, prop->key);

        const SerdNode p = serd_node_from_string(SERD_URI, USTR(key));
        if (prop->type == state->atom_Path && !dir) {
            const char* path = (const char*)prop->value;
            const char* abs_path = lilv_state_rel2abs(state, path);
            LILV_WARNF("Writing absolute path %s\n", abs_path);
            sratom_write(sratom, unmap, flags,
                subject, &p, prop->type,
                strlen(abs_path) + 1, abs_path);
        }
        else if (prop->flags & LV2_STATE_IS_POD ||
            prop->type == state->atom_Path) {
            sratom_write(sratom, unmap, flags,
                subject, &p, prop->type, prop->size, prop->value);
        }
        else {
            LILV_WARNF("Lost non-POD property <%s> on save\n", key);
        }
    }
}

static int
lilv_state_write(LilvWorld* world,
    LV2_URID_Map* map,
    LV2_URID_Unmap* unmap,
    const LilvState* state,
    SerdWriter* writer,
    const char* uri,
    const char* dir)
{
    SerdNode lv2_appliesTo = serd_node_from_string(
        SERD_CURIE, USTR("lv2:appliesTo"));

    const SerdNode* plugin_uri = sord_node_to_serd_node(
        state->plugin_uri->node);

    SerdNode subject = serd_node_from_string(SERD_URI, USTR(uri ? uri : ""));

    // <subject> a pset:Preset
    SerdNode p = serd_node_from_string(SERD_URI, USTR(LILV_NS_RDF "type"));
    SerdNode o = serd_node_from_string(SERD_URI, USTR(LV2_PRESETS__Preset));
    serd_writer_write_statement(writer, 0, NULL,
        &subject, &p, &o, NULL, NULL);

    // <subject> lv2:appliesTo <http://example.org/plugin>
    serd_writer_write_statement(writer, 0, NULL,
        &subject,
        &lv2_appliesTo,
        plugin_uri, NULL, NULL);

    // <subject> rdfs:label label
    if (state->label) {
        p = serd_node_from_string(SERD_URI, USTR(LILV_NS_RDFS "label"));
        o = serd_node_from_string(SERD_LITERAL, USTR(state->label));
        serd_writer_write_statement(writer, 0,
            NULL, &subject, &p, &o, NULL, NULL);
    }

    SerdEnv* env = serd_writer_get_env(writer);
    const SerdNode* base = serd_env_get_base_uri(env, NULL);

    Sratom* sratom = sratom_new(map);
    sratom_set_sink(sratom, (const char*)base->buf,
        (SerdStatementSink)serd_writer_write_statement,
        (SerdEndSink)serd_writer_end_anon,
        writer);

    // Write metadata
    sratom_set_pretty_numbers(sratom, false);  // Use precise types
    write_property_array(state, &state->metadata, sratom, 0,
        &subject, unmap, dir);

    // Write port values
    sratom_set_pretty_numbers(sratom, true);  // Use pretty numbers
    for (uint32_t i = 0; i < state->n_values; ++i) {
        PortValue* const value = &state->values[i];

        const SerdNode port = serd_node_from_string(
            SERD_BLANK, USTR(value->symbol));

        // <> lv2:port _:symbol
        p = serd_node_from_string(SERD_URI, USTR(LV2_CORE__port));
        serd_writer_write_statement(writer, SERD_ANON_O_BEGIN,
            NULL, &subject, &p, &port, NULL, NULL);

        // _:symbol lv2:symbol "symbol"
        p = serd_node_from_string(SERD_URI, USTR(LV2_CORE__symbol));
        o = serd_node_from_string(SERD_LITERAL, USTR(value->symbol));
        serd_writer_write_statement(writer, SERD_ANON_CONT,
            NULL, &port, &p, &o, NULL, NULL);

        // _:symbol pset:value value
        p = serd_node_from_string(SERD_URI, USTR(LV2_PRESETS__value));
        sratom_write(sratom, unmap, SERD_ANON_CONT, &port, &p,
            value->atom->type, value->atom->size, value->atom + 1);

        serd_writer_end_anon(writer, &port);
    }

    // Write properties
    const SerdNode body = serd_node_from_string(SERD_BLANK, USTR("body"));
    if (state->props.n > 0) {
        p = serd_node_from_string(SERD_URI, USTR(LV2_STATE__state));
        serd_writer_write_statement(writer, SERD_ANON_O_BEGIN, NULL,
            &subject, &p, &body, NULL, NULL);
    }
    sratom_set_pretty_numbers(sratom, false);  // Use precise types
    write_property_array(state, &state->props, sratom, SERD_ANON_CONT,
        &body, unmap, dir);

    if (state->props.n > 0) {
        serd_writer_end_anon(writer, &body);
    }

    sratom_free(sratom);
    return 0;
}

static void
lilv_state_make_links(const LilvState* state, const char* dir)
{
    // Create symlinks to files
    for (ZixTreeIter* i = zix_tree_begin(state->abs2rel);
        i != zix_tree_end(state->abs2rel);
        i = zix_tree_iter_next(i)) {
        const PathMap* pm = (const PathMap*)zix_tree_get(i);

        char* path = lilv_path_join(dir, pm->rel);
        if (lilv_path_is_child(pm->abs, state->copy_dir)
            && strcmp(state->copy_dir, dir)) {
            // Link directly to snapshot in the copy directory
            char* target = lilv_path_relative_to(pm->abs, dir);
            maybe_symlink(target, path);
            free(target);
        }
        else if (!lilv_path_is_child(pm->abs, dir)) {
            const char* link_dir = state->link_dir ? state->link_dir : dir;
            char* pat = lilv_path_join(link_dir, pm->rel);
            if (!strcmp(dir, link_dir)) {
                // Link directory is save directory, make link at exact path
                remove(pat);
                maybe_symlink(pm->abs, pat);
            }
            else {
                // Make a link in the link directory to external file
                char* lpath = lilv_find_free_path(pat, link_exists, pm->abs);
                if (!lilv_path_exists(lpath, NULL)) {
                    lilv_symlink(pm->abs, lpath);
                }

                // Make a link in the save directory to the external link
                char* target = lilv_path_relative_to(lpath, dir);
                maybe_symlink(target, path);
                free(target);
                free(lpath);
            }
            free(pat);
        }
        free(path);
    }
}

int
lilv_state_save(LilvWorld* world,
    LV2_URID_Map* map,
    LV2_URID_Unmap* unmap,
    const LilvState* state,
    const char* uri,
    const char* dir,
    const char* filename)
{
    if (!filename || !dir || lilv_mkdir_p(dir)) {
        return 1;
    }

    char* abs_dir = absolute_dir(dir);
    char* const path = lilv_path_join(abs_dir, filename);
    FILE* fd = fopen(path, "w");
    if (!fd) {
        LILV_ERRORF("Failed to open %s (%s)\n", path, strerror(errno));
        free(abs_dir);
        free(path);
        return 4;
    }

    // Create symlinks to files if necessary
    lilv_state_make_links(state, abs_dir);

    // Write state to Turtle file
    SerdNode    file = serd_node_new_file_uri(USTR(path), NULL, NULL, true);
    SerdNode    node = uri ? serd_node_from_string(SERD_URI, USTR(uri)) : file;
    SerdEnv* env = NULL;
    SerdWriter* ttl = ttl_file_writer(fd, &file, &env);
    int         ret = lilv_state_write(
        world, map, unmap, state, ttl, (const char*)node.buf, dir);

    // Set saved dir and uri (FIXME: const violation)
    free(state->dir);
    lilv_node_free(state->uri);
    ((LilvState*)state)->dir = lilv_strdup(abs_dir);
    ((LilvState*)state)->uri = lilv_new_uri(world, (const char*)node.buf);

    serd_node_free(&file);
    serd_writer_free(ttl);
    serd_env_free(env);
    fclose(fd);

    // Add entry to manifest
    char* const manifest = lilv_path_join(abs_dir, "manifest.ttl");
    add_state_to_manifest(world, state->plugin_uri, manifest, uri, path);

    free(manifest);
    free(abs_dir);
    free(path);
    return ret;
}

char*
lilv_state_to_string(LilvWorld* world,
    LV2_URID_Map* map,
    LV2_URID_Unmap* unmap,
    const LilvState* state,
    const char* uri,
    const char* base_uri)
{
    if (!uri) {
        LILV_ERROR("Attempt to serialise state with no URI\n");
        return NULL;
    }

    SerdChunk   chunk = { NULL, 0 };
    SerdEnv* env = NULL;
    SerdNode    base = serd_node_from_string(SERD_URI, USTR(base_uri));
    SerdWriter* writer = ttl_writer(serd_chunk_sink, &chunk, &base, &env);

    lilv_state_write(world, map, unmap, state, writer, uri, NULL);

    serd_writer_free(writer);
    serd_env_free(env);
    char* str = (char*)serd_chunk_sink_finish(&chunk);
    char* result = lilv_strdup(str);
    serd_free(str);
    return result;
}

static void
try_unlink(const char* state_dir, const char* path)
{
    if (!strncmp(state_dir, path, strlen(state_dir))) {
        if (lilv_path_exists(path, NULL) && unlink(path)) {
            LILV_ERRORF("Failed to remove %s (%s)\n", path, strerror(errno));
        }
    }
}

int
lilv_state_delete(LilvWorld* world,
    const LilvState* state)
{
    if (!state->dir) {
        LILV_ERROR("Attempt to delete unsaved state\n");
        return -1;
    }

    LilvNode* bundle = lilv_new_file_uri(world, NULL, state->dir);
    LilvNode* manifest = lilv_world_get_manifest_uri(world, bundle);
    char* manifest_path = lilv_node_get_path(manifest, NULL);
    const bool has_manifest = lilv_path_exists(manifest_path, NULL);
    SordModel* model = sord_new(world->world, SORD_SPO, false);

    if (has_manifest) {
        // Read manifest into temporary local model
        SerdEnv* env = serd_env_new(sord_node_to_serd_node(manifest->node));
        SerdReader* ttl = sord_new_reader(model, env, SERD_TURTLE, NULL);
        serd_reader_read_file(ttl, USTR(manifest_path));
        serd_reader_free(ttl);
        serd_env_free(env);
    }

    if (state->uri) {
        SordNode* file = sord_get(
            model, state->uri->node, world->uris.rdfs_seeAlso, NULL, NULL);
        if (file) {
            // Remove state file
            const uint8_t* uri = sord_node_get_string(file);
            char* path = (char*)serd_file_uri_parse(uri, NULL);
            try_unlink(state->dir, path);
            serd_free(path);
        }

        // Remove any existing manifest entries for this state
        const char* state_uri_str = lilv_node_as_string(state->uri);
        remove_manifest_entry(world->world, model, state_uri_str);
        remove_manifest_entry(world->world, world->model, state_uri_str);
    }

    // Drop bundle from model
    lilv_world_unload_bundle(world, bundle);

    if (sord_num_quads(model) == 0) {
        // Manifest is empty, attempt to remove bundle entirely
        if (has_manifest) {
            try_unlink(state->dir, manifest_path);
        }

        // Remove all known files from state bundle
        if (state->abs2rel) {
            // State created from instance, get paths from map
            for (ZixTreeIter* i = zix_tree_begin(state->abs2rel);
                i != zix_tree_end(state->abs2rel);
                i = zix_tree_iter_next(i)) {
                const PathMap* pm = (const PathMap*)zix_tree_get(i);
                char* path = lilv_path_join(state->dir, pm->rel);
                try_unlink(state->dir, path);
                free(path);
            }
        }
        else {
            // State loaded from model, get paths from loaded properties
            for (uint32_t i = 0; i < state->props.n; ++i) {
                const Property* const p = &state->props.props[i];
                if (p->type == state->atom_Path) {
                    try_unlink(state->dir, (const char*)p->value);
                }
            }
        }

        if (rmdir(state->dir)) {
            LILV_ERRORF("Failed to remove directory %s (%s)\n",
                state->dir, strerror(errno));
        }
    }
    else {
        // Still something in the manifest, update and reload bundle
        const SerdNode* manifest_node = sord_node_to_serd_node(manifest->node);
        SerdEnv* env = serd_env_new(manifest_node);

        write_manifest(world, env, model, manifest_node);
        lilv_world_load_bundle(world, bundle);
        serd_env_free(env);
    }

    sord_free(model);
    lilv_free(manifest_path);
    lilv_node_free(manifest);
    lilv_node_free(bundle);

    return 0;
}

static void
free_property_array(LilvState* state, PropertyArray* array)
{
    for (uint32_t i = 0; i < array->n; ++i) {
        Property* prop = &array->props[i];
        if ((prop->flags & LV2_STATE_IS_POD) ||
            prop->type == state->atom_Path) {
            free(prop->value);
        }
    }
    free(array->props);
}

void
lilv_state_free(LilvState* state)
{
    if (state) {
        free_property_array(state, &state->props);
        free_property_array(state, &state->metadata);
        for (uint32_t i = 0; i < state->n_values; ++i) {
            free(state->values[i].atom);
            free(state->values[i].symbol);
        }
        lilv_node_free(state->plugin_uri);
        lilv_node_free(state->uri);
        zix_tree_free(state->abs2rel);
        zix_tree_free(state->rel2abs);
        free(state->values);
        free(state->label);
        free(state->dir);
        free(state->scratch_dir);
        free(state->copy_dir);
        free(state->link_dir);
        free(state);
    }
}

bool
lilv_state_equals(const LilvState* a, const LilvState* b)
{
    if (!lilv_node_equals(a->plugin_uri, b->plugin_uri)
        || (a->label && !b->label)
        || (b->label && !a->label)
        || (a->label && b->label && strcmp(a->label, b->label))
        || a->props.n != b->props.n
        || a->n_values != b->n_values) {
        return false;
    }

    for (uint32_t i = 0; i < a->n_values; ++i) {
        PortValue* const av = &a->values[i];
        PortValue* const bv = &b->values[i];
        if (av->atom->size != bv->atom->size ||
            av->atom->type != bv->atom->type ||
            strcmp(av->symbol, bv->symbol) ||
            memcmp(av->atom + 1, bv->atom + 1, av->atom->size)) {
            return false;
        }
    }

    for (uint32_t i = 0; i < a->props.n; ++i) {
        Property* const ap = &a->props.props[i];
        Property* const bp = &b->props.props[i];
        if (ap->key != bp->key
            || ap->type != bp->type
            || ap->flags != bp->flags) {
            return false;
        }
        else if (ap->type == a->atom_Path) {
            if (!lilv_file_equals(lilv_state_rel2abs(a, (char*)ap->value),
                lilv_state_rel2abs(b, (char*)bp->value))) {
                return false;
            }
        }
        else if (ap->size != bp->size
            || memcmp(ap->value, bp->value, ap->size)) {
            return false;
        }
    }

    return true;
}

unsigned
lilv_state_get_num_properties(const LilvState* state)
{
    return state->props.n;
}

const LilvNode*
lilv_state_get_plugin_uri(const LilvState* state)
{
    return state->plugin_uri;
}

const LilvNode*
lilv_state_get_uri(const LilvState* state)
{
    return state->uri;
}

const char*
lilv_state_get_label(const LilvState* state)
{
    return state->label;
}

void
lilv_state_set_label(LilvState* state, const char* label)
{
    const size_t len = strlen(label);
    state->label = (char*)realloc(state->label, len + 1);
    memcpy(state->label, label, len + 1);
}

int
lilv_state_set_metadata(LilvState* state,
    uint32_t    key,
    const void* value,
    size_t      size,
    uint32_t    type,
    uint32_t    flags)
{
    append_property(state, &state->metadata, key, value, size, type, flags);
    return LV2_STATE_SUCCESS;
}


LilvUI*
lilv_ui_new(LilvWorld* world,
    LilvNode* uri,
    LilvNode* type_uri,
    LilvNode* binary_uri)
{
    assert(uri);
    assert(type_uri);
    assert(binary_uri);

    LilvUI* ui = (LilvUI*)malloc(sizeof(LilvUI));
    ui->world = world;
    ui->uri = uri;
    ui->binary_uri = binary_uri;

    // FIXME: kludge
    char* bundle = lilv_strdup(lilv_node_as_string(ui->binary_uri));
    char* last_slash = strrchr(bundle, '/') + 1;
    *last_slash = '\0';
    ui->bundle_uri = lilv_new_uri(world, bundle);
    free(bundle);

    ui->classes = lilv_nodes_new();
    zix_tree_insert((ZixTree*)ui->classes, type_uri, NULL);

    return ui;
}

void
lilv_ui_free(LilvUI* ui)
{
    lilv_node_free(ui->uri);
    lilv_node_free(ui->bundle_uri);
    lilv_node_free(ui->binary_uri);
    lilv_nodes_free(ui->classes);
    free(ui);
}

const LilvNode*
lilv_ui_get_uri(const LilvUI* ui)
{
    return ui->uri;
}

unsigned
lilv_ui_is_supported(const LilvUI* ui,
    LilvUISupportedFunc supported_func,
    const LilvNode* container_type,
    const LilvNode** ui_type)
{
    const LilvNodes* classes = lilv_ui_get_classes(ui);
    LILV_FOREACH(nodes, c, classes) {
        const LilvNode* type = lilv_nodes_get(classes, c);
        const unsigned  q = supported_func(lilv_node_as_uri(container_type),
            lilv_node_as_uri(type));
        if (q) {
            if (ui_type) {
                *ui_type = type;
            }
            return q;
        }
    }

    return 0;
}

const LilvNodes*
lilv_ui_get_classes(const LilvUI* ui)
{
    return ui->classes;
}

bool
lilv_ui_is_a(const LilvUI* ui, const LilvNode* class_uri)
{
    return lilv_nodes_contains(ui->classes, class_uri);
}

const LilvNode*
lilv_ui_get_bundle_uri(const LilvUI* ui)
{
    return ui->bundle_uri;
}

const LilvNode*
lilv_ui_get_binary_uri(const LilvUI* ui)
{
    return ui->binary_uri;
}


static int
lilv_world_drop_graph(LilvWorld* world, const SordNode* graph);

LilvWorld*
lilv_world_new(void)
{
    LilvWorld* world = (LilvWorld*)calloc(1, sizeof(LilvWorld));

    world->world = sord_world_new();
    if (!world->world) {
        goto fail;
    }

    world->model = sord_new(world->world, SORD_SPO | SORD_OPS, true);
    if (!world->model) {
        goto fail;
    }

    world->specs = NULL;
    world->plugin_classes = lilv_plugin_classes_new();
    world->plugins = lilv_plugins_new();
    world->zombies = lilv_plugins_new();
    world->loaded_files = zix_tree_new(
        false, lilv_resource_node_cmp, NULL, (ZixDestroyFunc)lilv_node_free);

    world->libs = zix_tree_new(false, lilv_lib_compare, NULL, NULL);

#define NS_DCTERMS "http://purl.org/dc/terms/"
#define NS_DYNMAN  "http://lv2plug.in/ns/ext/dynmanifest#"
#define NS_OWL     "http://www.w3.org/2002/07/owl#"

#define NEW_URI(uri) sord_new_uri(world->world, (const uint8_t*)(uri))

    world->uris.dc_replaces = NEW_URI(NS_DCTERMS   "replaces");
    world->uris.dman_DynManifest = NEW_URI(NS_DYNMAN    "DynManifest");
    world->uris.doap_name = NEW_URI(LILV_NS_DOAP "name");
    world->uris.lv2_Plugin = NEW_URI(LV2_CORE__Plugin);
    world->uris.lv2_Specification = NEW_URI(LV2_CORE__Specification);
    world->uris.lv2_appliesTo = NEW_URI(LV2_CORE__appliesTo);
    world->uris.lv2_binary = NEW_URI(LV2_CORE__binary);
    world->uris.lv2_default = NEW_URI(LV2_CORE__default);
    world->uris.lv2_designation = NEW_URI(LV2_CORE__designation);
    world->uris.lv2_extensionData = NEW_URI(LV2_CORE__extensionData);
    world->uris.lv2_index = NEW_URI(LV2_CORE__index);
    world->uris.lv2_latency = NEW_URI(LV2_CORE__latency);
    world->uris.lv2_maximum = NEW_URI(LV2_CORE__maximum);
    world->uris.lv2_microVersion = NEW_URI(LV2_CORE__microVersion);
    world->uris.lv2_minimum = NEW_URI(LV2_CORE__minimum);
    world->uris.lv2_minorVersion = NEW_URI(LV2_CORE__minorVersion);
    world->uris.lv2_name = NEW_URI(LV2_CORE__name);
    world->uris.lv2_optionalFeature = NEW_URI(LV2_CORE__optionalFeature);
    world->uris.lv2_port = NEW_URI(LV2_CORE__port);
    world->uris.lv2_portProperty = NEW_URI(LV2_CORE__portProperty);
    world->uris.lv2_reportsLatency = NEW_URI(LV2_CORE__reportsLatency);
    world->uris.lv2_requiredFeature = NEW_URI(LV2_CORE__requiredFeature);
    world->uris.lv2_symbol = NEW_URI(LV2_CORE__symbol);
    world->uris.lv2_prototype = NEW_URI(LV2_CORE__prototype);
    world->uris.owl_Ontology = NEW_URI(NS_OWL "Ontology");
    world->uris.pset_value = NEW_URI(LV2_PRESETS__value);
    world->uris.rdf_a = NEW_URI(LILV_NS_RDF  "type");
    world->uris.rdf_value = NEW_URI(LILV_NS_RDF  "value");
    world->uris.rdfs_Class = NEW_URI(LILV_NS_RDFS "Class");
    world->uris.rdfs_label = NEW_URI(LILV_NS_RDFS "label");
    world->uris.rdfs_seeAlso = NEW_URI(LILV_NS_RDFS "seeAlso");
    world->uris.rdfs_subClassOf = NEW_URI(LILV_NS_RDFS "subClassOf");
    world->uris.xsd_base64Binary = NEW_URI(LILV_NS_XSD  "base64Binary");
    world->uris.xsd_boolean = NEW_URI(LILV_NS_XSD  "boolean");
    world->uris.xsd_decimal = NEW_URI(LILV_NS_XSD  "decimal");
    world->uris.xsd_double = NEW_URI(LILV_NS_XSD  "double");
    world->uris.xsd_integer = NEW_URI(LILV_NS_XSD  "integer");
    world->uris.null_uri = NULL;

    world->lv2_plugin_class = lilv_plugin_class_new(
        world, NULL, world->uris.lv2_Plugin, "Plugin");
    assert(world->lv2_plugin_class);

    world->n_read_files = 0;
    world->opt.filter_language = true;
    world->opt.dyn_manifest = true;

    return world;

fail:
    /* keep on rockin' in the */ free(world);
    return NULL;
}

void
lilv_world_free(LilvWorld* world)
{
    if (!world) {
        return;
    }

    lilv_plugin_class_free(world->lv2_plugin_class);
    world->lv2_plugin_class = NULL;

    for (SordNode** n = (SordNode**)&world->uris; *n; ++n) {
        sord_node_free(world->world, *n);
    }

    for (LilvSpec* spec = world->specs; spec;) {
        LilvSpec* next = spec->next;
        sord_node_free(world->world, spec->spec);
        sord_node_free(world->world, spec->bundle);
        lilv_nodes_free(spec->data_uris);
        free(spec);
        spec = next;
    }
    world->specs = NULL;

    LILV_FOREACH(plugins, i, world->plugins) {
        const LilvPlugin* p = lilv_plugins_get(world->plugins, i);
        lilv_plugin_free((LilvPlugin*)p);
    }
    zix_tree_free((ZixTree*)world->plugins);
    world->plugins = NULL;

    LILV_FOREACH(plugins, i, world->zombies) {
        const LilvPlugin* p = lilv_plugins_get(world->zombies, i);
        lilv_plugin_free((LilvPlugin*)p);
    }
    zix_tree_free((ZixTree*)world->zombies);
    world->zombies = NULL;

    zix_tree_free((ZixTree*)world->loaded_files);
    world->loaded_files = NULL;

    zix_tree_free(world->libs);
    world->libs = NULL;

    zix_tree_free((ZixTree*)world->plugin_classes);
    world->plugin_classes = NULL;

    sord_free(world->model);
    world->model = NULL;

    sord_world_free(world->world);
    world->world = NULL;

    free(world->opt.lv2_path);
    free(world);
}

void
lilv_world_set_option(LilvWorld* world,
    const char* uri,
    const LilvNode* value)
{
    if (!strcmp(uri, LILV_OPTION_DYN_MANIFEST)) {
        if (lilv_node_is_bool(value)) {
            world->opt.dyn_manifest = lilv_node_as_bool(value);
            return;
        }
    }
    else if (!strcmp(uri, LILV_OPTION_FILTER_LANG)) {
        if (lilv_node_is_bool(value)) {
            world->opt.filter_language = lilv_node_as_bool(value);
            return;
        }
    }
    else if (!strcmp(uri, LILV_OPTION_LV2_PATH)) {
        if (lilv_node_is_string(value)) {
            world->opt.lv2_path = lilv_strdup(lilv_node_as_string(value));
            return;
        }
    }
    LILV_WARNF("Unrecognized or invalid option `%s'\n", uri);
}

LilvNodes*
lilv_world_find_nodes(LilvWorld* world,
    const LilvNode* subject,
    const LilvNode* predicate,
    const LilvNode* object)
{
    if (subject && !lilv_node_is_uri(subject) && !lilv_node_is_blank(subject)) {
        LILV_ERRORF("Subject `%s' is not a resource\n",
            sord_node_get_string(subject->node));
        return NULL;
    }
    else if (!predicate) {
        LILV_ERROR("Missing required predicate\n");
        return NULL;
    }
    else if (!lilv_node_is_uri(predicate)) {
        LILV_ERRORF("Predicate `%s' is not a URI\n",
            sord_node_get_string(predicate->node));
        return NULL;
    }
    else if (!subject && !object) {
        LILV_ERROR("Both subject and object are NULL\n");
        return NULL;
    }

    return lilv_world_find_nodes_internal(world,
        subject ? subject->node : NULL,
        predicate->node,
        object ? object->node : NULL);
}

LilvNode*
lilv_world_get(LilvWorld* world,
    const LilvNode* subject,
    const LilvNode* predicate,
    const LilvNode* object)
{
    SordNode* snode = sord_get(world->model,
        subject ? subject->node : NULL,
        predicate ? predicate->node : NULL,
        object ? object->node : NULL,
        NULL);
    LilvNode* lnode = lilv_node_new_from_node(world, snode);
    sord_node_free(world->world, snode);
    return lnode;
}

SordIter*
lilv_world_query_internal(LilvWorld* world,
    const SordNode* subject,
    const SordNode* predicate,
    const SordNode* object)
{
    return sord_search(world->model, subject, predicate, object, NULL);
}

bool
lilv_world_ask_internal(LilvWorld* world,
    const SordNode* subject,
    const SordNode* predicate,
    const SordNode* object)
{
    return sord_ask(world->model, subject, predicate, object, NULL);
}

bool
lilv_world_ask(LilvWorld* world,
    const LilvNode* subject,
    const LilvNode* predicate,
    const LilvNode* object)
{
    return sord_ask(world->model,
        subject ? subject->node : NULL,
        predicate ? predicate->node : NULL,
        object ? object->node : NULL,
        NULL);
}

SordModel*
lilv_world_filter_model(LilvWorld* world,
    SordModel* model,
    const SordNode* subject,
    const SordNode* predicate,
    const SordNode* object,
    const SordNode* graph)
{
    SordModel* results = sord_new(world->world, SORD_SPO, false);
    SordIter* i = sord_search(model, subject, predicate, object, graph);
    for (; !sord_iter_end(i); sord_iter_next(i)) {
        SordQuad quad;
        sord_iter_get(i, quad);
        sord_add(results, quad);
    }
    sord_iter_free(i);
    return results;
}

LilvNodes*
lilv_world_find_nodes_internal(LilvWorld* world,
    const SordNode* subject,
    const SordNode* predicate,
    const SordNode* object)
{
    return lilv_nodes_from_stream_objects(
        world,
        lilv_world_query_internal(world, subject, predicate, object),
        (object == NULL) ? SORD_OBJECT : SORD_SUBJECT);
}

static SerdNode
lilv_new_uri_relative_to_base(const uint8_t* uri_str,
    const uint8_t* base_uri_str)
{
    SerdURI base_uri;
    serd_uri_parse(base_uri_str, &base_uri);
    return serd_node_new_uri_from_string(uri_str, &base_uri, NULL);
}

const uint8_t*
lilv_world_blank_node_prefix(LilvWorld* world)
{
    static char str[32];
    snprintf(str, sizeof(str), "%d", world->n_read_files++);
    return (const uint8_t*)str;
}

/** Comparator for sequences (e.g. world->plugins). */
int
lilv_header_compare_by_uri(const void* a, const void* b, void* user_data)
{
    const struct LilvHeader* const header_a = (const struct LilvHeader*)a;
    const struct LilvHeader* const header_b = (const struct LilvHeader*)b;
    return strcmp(lilv_node_as_uri(header_a->uri),
        lilv_node_as_uri(header_b->uri));
}

/**
   Comparator for libraries (world->libs).

   Libraries do have a LilvHeader, but we must also compare the bundle to
   handle the case where the same library is loaded with different bundles, and
   consequently different contents (mainly plugins).
 */
int
lilv_lib_compare(const void* a, const void* b, void* user_data)
{
    const LilvLib* const lib_a = (const LilvLib*)a;
    const LilvLib* const lib_b = (const LilvLib*)b;
    int cmp = strcmp(lilv_node_as_uri(lib_a->uri),
        lilv_node_as_uri(lib_b->uri));
    return cmp ? cmp : strcmp(lib_a->bundle_path, lib_b->bundle_path);
}

/** Get an element of a collection of any object with an LilvHeader by URI. */
static ZixTreeIter*
lilv_collection_find_by_uri(const ZixTree* seq, const LilvNode* uri)
{
    ZixTreeIter* i = NULL;
    if (lilv_node_is_uri(uri)) {
        struct LilvHeader key = { NULL, (LilvNode*)uri };
        zix_tree_find(seq, &key, &i);
    }
    return i;
}

/** Get an element of a collection of any object with an LilvHeader by URI. */
struct LilvHeader*
    lilv_collection_get_by_uri(const ZixTree* seq, const LilvNode* uri)
{
    ZixTreeIter* const i = lilv_collection_find_by_uri(seq, uri);

    return i ? (struct LilvHeader*)zix_tree_get(i) : NULL;
}

static void
lilv_world_add_spec(LilvWorld* world,
    const SordNode* specification_node,
    const SordNode* bundle_node)
{
    LilvSpec* spec = (LilvSpec*)malloc(sizeof(LilvSpec));
    spec->spec = sord_node_copy(specification_node);
    spec->bundle = sord_node_copy(bundle_node);
    spec->data_uris = lilv_nodes_new();

    // Add all data files (rdfs:seeAlso)
    SordIter* files = sord_search(world->model,
        specification_node,
        world->uris.rdfs_seeAlso,
        NULL,
        NULL);
    FOREACH_MATCH(files) {
        const SordNode* file_node = sord_iter_get_node(files, SORD_OBJECT);
        zix_tree_insert((ZixTree*)spec->data_uris,
            lilv_node_new_from_node(world, file_node),
            NULL);
    }
    sord_iter_free(files);

    // Add specification to world specification list
    spec->next = world->specs;
    world->specs = spec;
}

static void
lilv_world_add_plugin(LilvWorld* world,
    const SordNode* plugin_node,
    const LilvNode* manifest_uri,
    void* dynmanifest,
    const SordNode* bundle)
{
    LilvNode* plugin_uri = lilv_node_new_from_node(world, plugin_node);
    ZixTreeIter* z = NULL;
    LilvPlugin* plugin = (LilvPlugin*)lilv_plugins_get_by_uri(
        world->plugins, plugin_uri);

    if (plugin) {
        // Existing plugin, if this is different bundle, ignore it
        // (use the first plugin found in LV2_PATH)
        const LilvNode* last_bundle = lilv_plugin_get_bundle_uri(plugin);
        const char* plugin_uri_str = lilv_node_as_uri(plugin_uri);
        if (sord_node_equals(bundle, last_bundle->node)) {
            LILV_WARNF("Reloading plugin <%s>\n", plugin_uri_str);
            plugin->loaded = false;
            lilv_node_free(plugin_uri);
        }
        else {
            LILV_WARNF("Duplicate plugin <%s>\n", plugin_uri_str);
            LILV_WARNF("... found in %s\n", lilv_node_as_string(last_bundle));
            LILV_WARNF("... and      %s (ignored)\n", sord_node_get_string(bundle));
            lilv_node_free(plugin_uri);
            return;
        }
    }
    else if ((z = lilv_collection_find_by_uri((const ZixTree*)world->zombies,
        plugin_uri))) {
        // Plugin bundle has been re-loaded, move from zombies to plugins
        plugin = (LilvPlugin*)zix_tree_get(z);
        zix_tree_remove((ZixTree*)world->zombies, z);
        zix_tree_insert((ZixTree*)world->plugins, plugin, NULL);
        lilv_node_free(plugin_uri);
        lilv_plugin_clear(plugin, lilv_node_new_from_node(world, bundle));
    }
    else {
        // Add new plugin to the world
        plugin = lilv_plugin_new(
            world, plugin_uri, lilv_node_new_from_node(world, bundle));

        // Add manifest as plugin data file (as if it were rdfs:seeAlso)
        zix_tree_insert((ZixTree*)plugin->data_uris,
            lilv_node_duplicate(manifest_uri),
            NULL);

        // Add plugin to world plugin sequence
        zix_tree_insert((ZixTree*)world->plugins, plugin, NULL);
    }


#ifdef LILV_DYN_MANIFEST
    // Set dynamic manifest library URI, if applicable
    if (dynmanifest) {
        plugin->dynmanifest = (LilvDynManifest*)dynmanifest;
        ++((LilvDynManifest*)dynmanifest)->refs;
    }
#endif

    // Add all plugin data files (rdfs:seeAlso)
    SordIter* files = sord_search(world->model,
        plugin_node,
        world->uris.rdfs_seeAlso,
        NULL,
        NULL);
    FOREACH_MATCH(files) {
        const SordNode* file_node = sord_iter_get_node(files, SORD_OBJECT);
        zix_tree_insert((ZixTree*)plugin->data_uris,
            lilv_node_new_from_node(world, file_node),
            NULL);
    }
    sord_iter_free(files);
}

SerdStatus
lilv_world_load_graph(LilvWorld* world, SordNode* graph, const LilvNode* uri)
{
    const SerdNode* base = sord_node_to_serd_node(uri->node);
    SerdEnv* env = serd_env_new(base);
    SerdReader* reader = sord_new_reader(
        world->model, env, SERD_TURTLE, graph);

    const SerdStatus st = lilv_world_load_file(world, reader, uri);

    serd_env_free(env);
    serd_reader_free(reader);
    return st;
}

static void
lilv_world_load_dyn_manifest(LilvWorld* world,
    SordNode* bundle_node,
    const LilvNode* manifest)
{
#ifdef LILV_DYN_MANIFEST
    if (!world->opt.dyn_manifest) {
        return;
    }

    LV2_Dyn_Manifest_Handle handle = NULL;

    // ?dman a dynman:DynManifest bundle_node
    SordModel* model = lilv_world_filter_model(world,
        world->model,
        NULL,
        world->uris.rdf_a,
        world->uris.dman_DynManifest,
        bundle_node);
    SordIter* iter = sord_begin(model);
    for (; !sord_iter_end(iter); sord_iter_next(iter)) {
        const SordNode* dmanifest = sord_iter_get_node(iter, SORD_SUBJECT);

        // ?dman lv2:binary ?binary
        SordIter* binaries = sord_search(world->model,
            dmanifest,
            world->uris.lv2_binary,
            NULL,
            bundle_node);
        if (sord_iter_end(binaries)) {
            sord_iter_free(binaries);
            LILV_ERRORF("Dynamic manifest in <%s> has no binaries, ignored\n",
                sord_node_get_string(bundle_node));
            continue;
        }

        // Get binary path
        const SordNode* binary = sord_iter_get_node(binaries, SORD_OBJECT);
        const uint8_t* lib_uri = sord_node_get_string(binary);
        char* lib_path = lilv_file_uri_parse((const char*)lib_uri, 0);
        if (!lib_path) {
            LILV_ERROR("No dynamic manifest library path\n");
            sord_iter_free(binaries);
            continue;
        }

        // Open library
        dlerror();
        void* lib = dlopen(lib_path, RTLD_LAZY);
        if (!lib) {
            LILV_ERRORF("Failed to open dynmanifest library `%s' (%s)\n",
                lib_path, dlerror());
            sord_iter_free(binaries);
            lilv_free(lib_path);
            continue;
        }

        // Open dynamic manifest
        typedef int (*OpenFunc)(LV2_Dyn_Manifest_Handle*,
            const LV2_Feature* const*);
        OpenFunc dmopen = (OpenFunc)lilv_dlfunc(lib, "lv2_dyn_manifest_open");
        if (!dmopen || dmopen(&handle, &dman_features)) {
            LILV_ERRORF("No `lv2_dyn_manifest_open' in `%s'\n", lib_path);
            sord_iter_free(binaries);
            dlclose(lib);
            lilv_free(lib_path);
            continue;
        }

        // Get subjects (the data that would be in manifest.ttl)
        typedef int (*GetSubjectsFunc)(LV2_Dyn_Manifest_Handle, FILE*);
        GetSubjectsFunc get_subjects_func = (GetSubjectsFunc)lilv_dlfunc(
            lib, "lv2_dyn_manifest_get_subjects");
        if (!get_subjects_func) {
            LILV_ERRORF("No `lv2_dyn_manifest_get_subjects' in `%s'\n",
                lib_path);
            sord_iter_free(binaries);
            dlclose(lib);
            lilv_free(lib_path);
            continue;
        }

        LilvDynManifest* desc = (LilvDynManifest*)malloc(sizeof(LilvDynManifest));
        desc->bundle = lilv_node_new_from_node(world, bundle_node);
        desc->lib = lib;
        desc->handle = handle;
        desc->refs = 0;

        sord_iter_free(binaries);

        // Generate data file
        FILE* fd = tmpfile();
        get_subjects_func(handle, fd);
        rewind(fd);

        // Parse generated data file into temporary model
        // FIXME
        const SerdNode* base = sord_node_to_serd_node(dmanifest);
        SerdEnv* env = serd_env_new(base);
        SerdReader* reader = sord_new_reader(
            world->model, env, SERD_TURTLE, sord_node_copy(dmanifest));
        serd_reader_add_blank_prefix(reader,
            lilv_world_blank_node_prefix(world));
        serd_reader_read_file_handle(reader, fd,
            (const uint8_t*)"(dyn-manifest)");
        serd_reader_free(reader);
        serd_env_free(env);

        // Close (and automatically delete) temporary data file
        fclose(fd);

        // ?plugin a lv2:Plugin
        SordModel* plugins = lilv_world_filter_model(world,
            world->model,
            NULL,
            world->uris.rdf_a,
            world->uris.lv2_Plugin,
            dmanifest);
        SordIter* p = sord_begin(plugins);
        FOREACH_MATCH(p) {
            const SordNode* plug = sord_iter_get_node(p, SORD_SUBJECT);
            lilv_world_add_plugin(world, plug, manifest, desc, bundle_node);
        }
        if (desc->refs == 0) {
            lilv_dynmanifest_free(desc);
        }
        sord_iter_free(p);
        sord_free(plugins);
        lilv_free(lib_path);
    }
    sord_iter_free(iter);
    sord_free(model);
#endif  // LILV_DYN_MANIFEST
}

#ifdef LILV_DYN_MANIFEST
void
lilv_dynmanifest_free(LilvDynManifest* dynmanifest)
{
    typedef int (*CloseFunc)(LV2_Dyn_Manifest_Handle);
    CloseFunc close_func = (CloseFunc)lilv_dlfunc(dynmanifest->lib,
        "lv2_dyn_manifest_close");
    if (close_func) {
        close_func(dynmanifest->handle);
    }

    dlclose(dynmanifest->lib);
    lilv_node_free(dynmanifest->bundle);
    free(dynmanifest);
}
#endif  // LILV_DYN_MANIFEST

LilvNode*
lilv_world_get_manifest_uri(LilvWorld* world, const LilvNode* bundle_uri)
{
    SerdNode manifest_uri = lilv_new_uri_relative_to_base(
        (const uint8_t*)"manifest.ttl",
        sord_node_get_string(bundle_uri->node));
    LilvNode* manifest = lilv_new_uri(world, (const char*)manifest_uri.buf);
    serd_node_free(&manifest_uri);
    return manifest;
}

static SordModel*
load_plugin_model(LilvWorld* world,
    const LilvNode* bundle_uri,
    const LilvNode* plugin_uri)
{
    // Create model and reader for loading into it
    SordNode* bundle_node = bundle_uri->node;
    SordModel* model = sord_new(world->world, SORD_SPO | SORD_OPS, false);
    SerdEnv* env = serd_env_new(sord_node_to_serd_node(bundle_node));
    SerdReader* reader = sord_new_reader(model, env, SERD_TURTLE, NULL);

    // Load manifest
    LilvNode* manifest_uri = lilv_world_get_manifest_uri(world, bundle_uri);
    serd_reader_add_blank_prefix(reader, lilv_world_blank_node_prefix(world));
    serd_reader_read_file(
        reader, (const uint8_t*)lilv_node_as_string(manifest_uri));

    // Load any seeAlso files
    SordModel* files = lilv_world_filter_model(
        world, model, plugin_uri->node, world->uris.rdfs_seeAlso, NULL, NULL);

    SordIter* f = sord_begin(files);
    FOREACH_MATCH(f) {
        const SordNode* file = sord_iter_get_node(f, SORD_OBJECT);
        const uint8_t* file_str = sord_node_get_string(file);
        if (sord_node_get_type(file) == SORD_URI) {
            serd_reader_add_blank_prefix(
                reader, lilv_world_blank_node_prefix(world));
            serd_reader_read_file(reader, file_str);
        }
    }

    sord_iter_free(f);
    sord_free(files);
    serd_reader_free(reader);
    serd_env_free(env);
    lilv_node_free(manifest_uri);

    return model;
}

static LilvVersion
get_version(LilvWorld* world, SordModel* model, const LilvNode* subject)
{
    const SordNode* minor_node = sord_get(
        model, subject->node, world->uris.lv2_minorVersion, NULL, NULL);
    const SordNode* micro_node = sord_get(
        model, subject->node, world->uris.lv2_microVersion, NULL, NULL);


    LilvVersion version = { 0, 0 };
    if (minor_node && micro_node) {
        version.minor = atoi((const char*)sord_node_get_string(minor_node));
        version.micro = atoi((const char*)sord_node_get_string(micro_node));
    }

    return version;
}

void
lilv_world_load_bundle(LilvWorld* world, const LilvNode* bundle_uri)
{
    if (!lilv_node_is_uri(bundle_uri)) {
        LILV_ERRORF("Bundle URI `%s' is not a URI\n",
            sord_node_get_string(bundle_uri->node));
        return;
    }

    SordNode* bundle_node = bundle_uri->node;
    LilvNode* manifest = lilv_world_get_manifest_uri(world, bundle_uri);

    // Read manifest into model with graph = bundle_node
    SerdStatus st = lilv_world_load_graph(world, bundle_node, manifest);
    if (st > SERD_FAILURE) {
        LILV_ERRORF("Error reading %s\n", lilv_node_as_string(manifest));
        lilv_node_free(manifest);
        return;
    }

    // ?plugin a lv2:Plugin
    SordIter* plug_results = sord_search(world->model,
        NULL,
        world->uris.rdf_a,
        world->uris.lv2_Plugin,
        bundle_node);

    // Find any loaded plugins that will be replaced with a newer version
    LilvNodes* unload_uris = lilv_nodes_new();
    FOREACH_MATCH(plug_results) {
        const SordNode* plug = sord_iter_get_node(plug_results, SORD_SUBJECT);

        LilvNode* plugin_uri = lilv_node_new_from_node(world, plug);
        const LilvPlugin* plugin = lilv_plugins_get_by_uri(world->plugins, plugin_uri);
        const LilvNode* last_bundle = plugin ? lilv_plugin_get_bundle_uri(plugin) : NULL;
        if (!plugin || sord_node_equals(bundle_node, last_bundle->node)) {
            // No previously loaded version, or it's from the same bundle
            lilv_node_free(plugin_uri);
            continue;
        }

        // Compare versions
        SordModel* this_model = load_plugin_model(world, bundle_uri, plugin_uri);
        LilvVersion this_version = get_version(world, this_model, plugin_uri);
        SordModel* last_model = load_plugin_model(world, last_bundle, plugin_uri);
        LilvVersion last_version = get_version(world, last_model, plugin_uri);
        sord_free(this_model);
        sord_free(last_model);
        const int cmp = lilv_version_cmp(&this_version, &last_version);
        if (cmp > 0) {
            zix_tree_insert((ZixTree*)unload_uris,
                lilv_node_duplicate(plugin_uri),
                NULL);
            LILV_WARNF("Replacing version %d.%d of <%s> from <%s>\n",
                last_version.minor, last_version.micro,
                sord_node_get_string(plug),
                sord_node_get_string(last_bundle->node));
            LILV_NOTEF("New version %d.%d found in <%s>\n",
                this_version.minor, this_version.micro,
                sord_node_get_string(bundle_node));
        }
        else if (cmp < 0) {
            LILV_WARNF("Ignoring bundle <%s>\n",
                sord_node_get_string(bundle_node));
            LILV_NOTEF("Newer version of <%s> loaded from <%s>\n",
                sord_node_get_string(plug),
                sord_node_get_string(last_bundle->node));
            lilv_node_free(plugin_uri);
            sord_iter_free(plug_results);
            lilv_world_drop_graph(world, bundle_node);
            lilv_node_free(manifest);
            lilv_nodes_free(unload_uris);
            return;
        }
        lilv_node_free(plugin_uri);
    }

    sord_iter_free(plug_results);

    // Unload any old conflicting plugins
    LilvNodes* unload_bundles = lilv_nodes_new();
    LILV_FOREACH(nodes, i, unload_uris) {
        const LilvNode* uri = lilv_nodes_get(unload_uris, i);
        const LilvPlugin* plugin = lilv_plugins_get_by_uri(world->plugins, uri);
        const LilvNode* bundle = lilv_plugin_get_bundle_uri(plugin);

        // Unload plugin and record bundle for later unloading
        lilv_world_unload_resource(world, uri);
        zix_tree_insert((ZixTree*)unload_bundles,
            lilv_node_duplicate(bundle),
            NULL);

    }
    lilv_nodes_free(unload_uris);

    // Now unload the associated bundles
    // This must be done last since several plugins could be in the same bundle
    LILV_FOREACH(nodes, i, unload_bundles) {
        lilv_world_unload_bundle(world, lilv_nodes_get(unload_bundles, i));
    }
    lilv_nodes_free(unload_bundles);

    // Re-search for plugin results now that old plugins are gone
    plug_results = sord_search(world->model,
        NULL,
        world->uris.rdf_a,
        world->uris.lv2_Plugin,
        bundle_node);

    FOREACH_MATCH(plug_results) {
        const SordNode* plug = sord_iter_get_node(plug_results, SORD_SUBJECT);
        lilv_world_add_plugin(world, plug, manifest, NULL, bundle_node);
    }
    sord_iter_free(plug_results);

    lilv_world_load_dyn_manifest(world, bundle_node, manifest);

    // ?spec a lv2:Specification
    // ?spec a owl:Ontology
    const SordNode* spec_preds[] = { world->uris.lv2_Specification,
                                     world->uris.owl_Ontology,
                                     NULL };
    for (const SordNode** p = spec_preds; *p; ++p) {
        SordIter* i = sord_search(
            world->model, NULL, world->uris.rdf_a, *p, bundle_node);
        FOREACH_MATCH(i) {
            const SordNode* spec = sord_iter_get_node(i, SORD_SUBJECT);
            lilv_world_add_spec(world, spec, bundle_node);
        }
        sord_iter_free(i);
    }

    lilv_node_free(manifest);
}

static int
lilv_world_drop_graph(LilvWorld* world, const SordNode* graph)
{
    SordIter* i = sord_search(world->model, NULL, NULL, NULL, graph);
    while (!sord_iter_end(i)) {
        const SerdStatus st = sord_erase(world->model, i);
        if (st) {
            LILV_ERRORF("Error removing statement from <%s> (%s)\n",
                sord_node_get_string(graph), serd_strerror(st));
            return st;
        }
    }
    sord_iter_free(i);

    return 0;
}

/** Remove loaded_files entry so file will be reloaded if requested. */
static int
lilv_world_unload_file(LilvWorld* world, const LilvNode* file)
{
    ZixTreeIter* iter;
    if (!zix_tree_find((ZixTree*)world->loaded_files, file, &iter)) {
        zix_tree_remove((ZixTree*)world->loaded_files, iter);
        return 0;
    }
    return 1;
}

int
lilv_world_unload_bundle(LilvWorld* world, const LilvNode* bundle_uri)
{
    if (!bundle_uri) {
        return 0;
    }

    // Find all loaded files that are inside the bundle
    LilvNodes* files = lilv_nodes_new();
    LILV_FOREACH(nodes, i, world->loaded_files) {
        const LilvNode* file = lilv_nodes_get(world->loaded_files, i);
        if (!strncmp(lilv_node_as_string(file),
            lilv_node_as_string(bundle_uri),
            strlen(lilv_node_as_string(bundle_uri)))) {
            zix_tree_insert((ZixTree*)files,
                lilv_node_duplicate(file),
                NULL);
        }
    }

    // Unload all loaded files in the bundle
    LILV_FOREACH(nodes, i, files) {
        const LilvNode* file = lilv_nodes_get(world->plugins, i);
        lilv_world_unload_file(world, file);
    }

    lilv_nodes_free(files);

    /* Remove any plugins in the bundle from the plugin list.  Since the
       application may still have a pointer to the LilvPlugin, it can not be
       destroyed here.  Instead, we move it to the zombie plugin list, so it
       will not be in the list returned by lilv_world_get_all_plugins() but can
       still be used.
    */
    ZixTreeIter* i = zix_tree_begin((ZixTree*)world->plugins);
    while (i != zix_tree_end((ZixTree*)world->plugins)) {
        LilvPlugin* p = (LilvPlugin*)zix_tree_get(i);
        ZixTreeIter* next = zix_tree_iter_next(i);

        if (lilv_node_equals(lilv_plugin_get_bundle_uri(p), bundle_uri)) {
            zix_tree_remove((ZixTree*)world->plugins, i);
            zix_tree_insert((ZixTree*)world->zombies, p, NULL);
        }

        i = next;
    }

    // Drop everything in bundle graph
    return lilv_world_drop_graph(world, bundle_uri->node);
}

static void
load_dir_entry(const char* dir, const char* name, void* data)
{
    LilvWorld* world = (LilvWorld*)data;
    if (!strcmp(name, ".") || !strcmp(name, "..")) {
        return;
    }

    char* path = lilv_strjoin(dir, "/", name, "/", NULL);
    SerdNode  suri = serd_node_new_file_uri((const uint8_t*)path, 0, 0, true);
    LilvNode* node = lilv_new_uri(world, (const char*)suri.buf);

    lilv_world_load_bundle(world, node);
    lilv_node_free(node);
    serd_node_free(&suri);
    free(path);
}

/** Load all bundles in the directory at `dir_path`. */
static void
lilv_world_load_directory(LilvWorld* world, const char* dir_path)
{
    char* path = lilv_expand(dir_path);
    if (path) {
        lilv_dir_for_each(path, world, load_dir_entry);
        free(path);
    }
}

static const char*
first_path_sep(const char* path)
{
    for (const char* p = path; *p != '\0'; ++p) {
        if (*p == LILV_PATH_SEP[0]) {
            return p;
        }
    }
    return NULL;
}

/** Load all bundles found in `lv2_path`.
 * @param lv2_path A colon-delimited list of directories.  These directories
 * should contain LV2 bundle directories (ie the search path is a list of
 * parent directories of bundles, not a list of bundle directories).
 */
static void
lilv_world_load_path(LilvWorld* world,
    const char* lv2_path)
{
    while (lv2_path[0] != '\0') {
        const char* const sep = first_path_sep(lv2_path);
        if (sep) {
            const size_t dir_len = sep - lv2_path;
            char* const  dir = (char*)malloc(dir_len + 1);
            memcpy(dir, lv2_path, dir_len);
            dir[dir_len] = '\0';
            lilv_world_load_directory(world, dir);
            free(dir);
            lv2_path += dir_len + 1;
        }
        else {
            lilv_world_load_directory(world, lv2_path);
            lv2_path = "\0";
        }
    }
}

void
lilv_world_load_specifications(LilvWorld* world)
{
    for (LilvSpec* spec = world->specs; spec; spec = spec->next) {
        LILV_FOREACH(nodes, f, spec->data_uris) {
            LilvNode* file = (LilvNode*)lilv_collection_get(spec->data_uris, f);
            lilv_world_load_graph(world, NULL, file);
        }
    }
}

void
lilv_world_load_plugin_classes(LilvWorld* world)
{
    /* FIXME: This loads all classes, not just lv2:Plugin subclasses.
       However, if the host gets all the classes via lilv_plugin_class_get_children
       starting with lv2:Plugin as the root (which is e.g. how a host would build
       a menu), they won't be seen anyway...
    */

    SordIter* classes = sord_search(world->model,
        NULL,
        world->uris.rdf_a,
        world->uris.rdfs_Class,
        NULL);
    FOREACH_MATCH(classes) {
        const SordNode* class_node = sord_iter_get_node(classes, SORD_SUBJECT);

        SordNode* parent = sord_get(
            world->model, class_node, world->uris.rdfs_subClassOf, NULL, NULL);
        if (!parent || sord_node_get_type(parent) != SORD_URI) {
            continue;
        }

        SordNode* label = sord_get(
            world->model, class_node, world->uris.rdfs_label, NULL, NULL);
        if (!label) {
            sord_node_free(world->world, parent);
            continue;
        }

        LilvPluginClass* pclass = lilv_plugin_class_new(
            world, parent, class_node,
            (const char*)sord_node_get_string(label));
        if (pclass) {
            zix_tree_insert((ZixTree*)world->plugin_classes, pclass, NULL);
        }

        sord_node_free(world->world, label);
        sord_node_free(world->world, parent);
    }
    sord_iter_free(classes);
}

void
lilv_world_load_all(LilvWorld* world)
{
    const char* lv2_path = world->opt.lv2_path;
    if (!lv2_path) {
        lv2_path = getenv("LV2_PATH");
    }
    if (!lv2_path) {
        lv2_path = LILV_DEFAULT_LV2_PATH;
    }

    // Discover bundles and read all manifest files into model
    lilv_world_load_path(world, lv2_path);

    LILV_FOREACH(plugins, p, world->plugins) {
        const LilvPlugin* plugin = (const LilvPlugin*)lilv_collection_get(
            (ZixTree*)world->plugins, p);

        // ?new dc:replaces plugin
        if (sord_ask(world->model,
            NULL,
            world->uris.dc_replaces,
            lilv_plugin_get_uri(plugin)->node,
            NULL)) {
            // TODO: Check if replacement is a known plugin? (expensive)
            ((LilvPlugin*)plugin)->replaced = true;
        }
    }

    // Query out things to cache
    lilv_world_load_specifications(world);
    lilv_world_load_plugin_classes(world);
}

SerdStatus
lilv_world_load_file(LilvWorld* world, SerdReader* reader, const LilvNode* uri)
{
    ZixTreeIter* iter;
    if (!zix_tree_find((ZixTree*)world->loaded_files, uri, &iter)) {
        return SERD_FAILURE;  // File has already been loaded
    }

    size_t               uri_len;
    const uint8_t* const uri_str = sord_node_get_string_counted(
        uri->node, &uri_len);
    if (strncmp((const char*)uri_str, "file:", 5)) {
        return SERD_FAILURE;  // Not a local file
    }
    else if (strcmp((const char*)uri_str + uri_len - 4, ".ttl")) {
        return SERD_FAILURE;  // Not a Turtle file
    }

    serd_reader_add_blank_prefix(reader, lilv_world_blank_node_prefix(world));
    const SerdStatus st = serd_reader_read_file(reader, uri_str);
    if (st) {
        LILV_ERRORF("Error loading file `%s'\n", lilv_node_as_string(uri));
        return st;
    }

    zix_tree_insert((ZixTree*)world->loaded_files,
        lilv_node_duplicate(uri),
        NULL);
    return SERD_SUCCESS;
}

int
lilv_world_load_resource(LilvWorld* world,
    const LilvNode* resource)
{
    if (!lilv_node_is_uri(resource) && !lilv_node_is_blank(resource)) {
        LILV_ERRORF("Node `%s' is not a resource\n",
            sord_node_get_string(resource->node));
        return -1;
    }

    SordModel* files = lilv_world_filter_model(world,
        world->model,
        resource->node,
        world->uris.rdfs_seeAlso,
        NULL, NULL);

    SordIter* f = sord_begin(files);
    int       n_read = 0;
    FOREACH_MATCH(f) {
        const SordNode* file = sord_iter_get_node(f, SORD_OBJECT);
        const uint8_t* file_str = sord_node_get_string(file);
        LilvNode* file_node = lilv_node_new_from_node(world, file);
        if (sord_node_get_type(file) != SORD_URI) {
            LILV_ERRORF("rdfs:seeAlso node `%s' is not a URI\n", file_str);
        }
        else if (!lilv_world_load_graph(world, (SordNode*)file, file_node)) {
            ++n_read;
        }
        lilv_node_free(file_node);
    }
    sord_iter_free(f);

    sord_free(files);
    return n_read;
}

int
lilv_world_unload_resource(LilvWorld* world,
    const LilvNode* resource)
{
    if (!lilv_node_is_uri(resource) && !lilv_node_is_blank(resource)) {
        LILV_ERRORF("Node `%s' is not a resource\n",
            sord_node_get_string(resource->node));
        return -1;
    }

    SordModel* files = lilv_world_filter_model(world,
        world->model,
        resource->node,
        world->uris.rdfs_seeAlso,
        NULL, NULL);

    SordIter* f = sord_begin(files);
    int       n_dropped = 0;
    FOREACH_MATCH(f) {
        const SordNode* file = sord_iter_get_node(f, SORD_OBJECT);
        LilvNode* file_node = lilv_node_new_from_node(world, file);
        if (sord_node_get_type(file) != SORD_URI) {
            LILV_ERRORF("rdfs:seeAlso node `%s' is not a URI\n",
                sord_node_get_string(file));
        }
        else if (!lilv_world_drop_graph(world, file_node->node)) {
            lilv_world_unload_file(world, file_node);
            ++n_dropped;
        }
        lilv_node_free(file_node);
    }
    sord_iter_free(f);

    sord_free(files);
    return n_dropped;
}

const LilvPluginClass*
lilv_world_get_plugin_class(const LilvWorld* world)
{
    return world->lv2_plugin_class;
}

const LilvPluginClasses*
lilv_world_get_plugin_classes(const LilvWorld* world)
{
    return world->plugin_classes;
}

const LilvPlugins*
lilv_world_get_all_plugins(const LilvWorld* world)
{
    return world->plugins;
}

LilvNode*
lilv_world_get_symbol(LilvWorld* world, const LilvNode* subject)
{
    // Check for explicitly given symbol
    SordNode* snode = sord_get(
        world->model, subject->node, world->uris.lv2_symbol, NULL, NULL);

    if (snode) {
        LilvNode* ret = lilv_node_new_from_node(world, snode);
        sord_node_free(world->world, snode);
        return ret;
    }

    if (!lilv_node_is_uri(subject)) {
        return NULL;
    }

    // Find rightmost segment of URI
    SerdURI uri;
    serd_uri_parse((const uint8_t*)lilv_node_as_uri(subject), &uri);
    const char* str = "_";
    if (uri.fragment.buf) {
        str = (const char*)uri.fragment.buf + 1;
    }
    else if (uri.query.buf) {
        str = (const char*)uri.query.buf;
    }
    else if (uri.path.buf) {
        const char* last_slash = strrchr((const char*)uri.path.buf, '/');
        str = last_slash ? (last_slash + 1) : (const char*)uri.path.buf;
    }

    // Replace invalid characters
    const size_t len = strlen(str);
    char* const  sym = (char*)calloc(1, len + 1);
    for (size_t i = 0; i < len; ++i) {
        const char c = str[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c == '_') || (i > 0 && c >= '0' && c <= '9'))) {
            sym[i] = '_';
        }
        else {
            sym[i] = str[i];
        }
    }

    LilvNode* ret = lilv_new_string(world, sym);
    free(sym);
    return ret;
}


typedef struct ZixTreeNodeImpl ZixTreeNode;

struct ZixTreeImpl {
    ZixTreeNode* root;
    ZixDestroyFunc destroy;
    ZixComparator  cmp;
    void* cmp_data;
    size_t         size;
    bool           allow_duplicates;
};

struct ZixTreeNodeImpl {
    void* data;
    struct ZixTreeNodeImpl* left;
    struct ZixTreeNodeImpl* right;
    struct ZixTreeNodeImpl* parent;
    int_fast8_t             balance;
};

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

// Uncomment these for debugging features
// #define ZIX_TREE_DUMP         1
// #define ZIX_TREE_VERIFY       1
// #define ZIX_TREE_HYPER_VERIFY 1

#if defined(ZIX_TREE_VERIFY) || defined(ZIX_TREE_HYPER_VERIFY)
#    include "tree_debug.h"
#    define ASSERT_BALANCE(n) assert(verify_balance(n))
#else
#    define ASSERT_BALANCE(n)
#endif

#ifdef ZIX_TREE_DUMP
#    include "tree_debug.h"
#    define DUMP(t) zix_tree_print(t->root, 0)
#    define DEBUG_PRINTF(fmt, ...) printf(fmt, __VA_ARGS__)
#else
#    define DUMP(t)
#    define DEBUG_PRINTF(fmt, ...)
#endif

ZixTree*
zix_tree_new(bool           allow_duplicates,
    ZixComparator  cmp,
    void* cmp_data,
    ZixDestroyFunc destroy)
{
    ZixTree* t = (ZixTree*)malloc(sizeof(ZixTree));
    t->root = NULL;
    t->destroy = destroy;
    t->cmp = cmp;
    t->cmp_data = cmp_data;
    t->size = 0;
    t->allow_duplicates = allow_duplicates;
    return t;
}

void
zix_tree_free_rec(ZixTree* t, ZixTreeNode* n)
{
    if (n) {
        zix_tree_free_rec(t, n->left);
        zix_tree_free_rec(t, n->right);
        if (t->destroy) {
            t->destroy(n->data);
        }
        free(n);
    }
}

void
zix_tree_free(ZixTree* t)
{
    if (t) {
        zix_tree_free_rec(t, t->root);
        free(t);
    }
}

size_t
zix_tree_size(const ZixTree* t)
{
    return t->size;
}

void
rotate(ZixTreeNode* p, ZixTreeNode* q)
{
    assert(q->parent == p);
    assert(p->left == q || p->right == q);

    q->parent = p->parent;
    if (q->parent) {
        if (q->parent->left == p) {
            q->parent->left = q;
        }
        else {
            q->parent->right = q;
        }
    }

    if (p->right == q) {
        // Rotate left
        p->right = q->left;
        q->left = p;
        if (p->right) {
            p->right->parent = p;
        }
    }
    else {
        // Rotate right
        assert(p->left == q);
        p->left = q->right;
        q->right = p;
        if (p->left) {
            p->left->parent = p;
        }
    }

    p->parent = q;
}

/**
 * Rotate left about `p`.
 *
 *    p              q
 *   / \            / \
 *  A   q    =>    p   C
 *     / \        / \
 *    B   C      A   B
 */
ZixTreeNode*
rotate_left(ZixTreeNode* p, int* height_change)
{
    ZixTreeNode* const q = p->right;
    *height_change = (q->balance == 0) ? 0 : -1;

    DEBUG_PRINTF("LL %ld\n", (intptr_t)p->data);

    assert(p->balance == 2);
    assert(q->balance == 0 || q->balance == 1);

    rotate(p, q);

    // p->balance -= 1 + MAX(0, q->balance);
    // q->balance -= 1 - MIN(0, p->balance);
    --q->balance;
    p->balance = -(q->balance);

    ASSERT_BALANCE(p);
    ASSERT_BALANCE(q);
    return q;
}

/**
 * Rotate right about `p`.
 *
 *      p          q
 *     / \        / \
 *    q   C  =>  A   p
 *   / \            / \
 *  A   B          B   C
 *
 */
ZixTreeNode*
rotate_right(ZixTreeNode* p, int* height_change)
{
    ZixTreeNode* const q = p->left;
    *height_change = (q->balance == 0) ? 0 : -1;

    DEBUG_PRINTF("RR %ld\n", (intptr_t)p->data);

    assert(p->balance == -2);
    assert(q->balance == 0 || q->balance == -1);

    rotate(p, q);

    // p->balance += 1 - MIN(0, q->balance);
    // q->balance += 1 + MAX(0, p->balance);
    ++q->balance;
    p->balance = -(q->balance);

    ASSERT_BALANCE(p);
    ASSERT_BALANCE(q);
    return q;
}

/**
 * Rotate left about `p->left` then right about `p`.
 *
 *      p             r
 *     / \           / \
 *    q   D  =>    q     p
 *   / \          / \   / \
 *  A   r        A   B C   D
 *     / \
 *    B   C
 *
 */
ZixTreeNode*
rotate_left_right(ZixTreeNode* p, int* height_change)
{
    ZixTreeNode* const q = p->left;
    ZixTreeNode* const r = q->right;

    assert(p->balance == -2);
    assert(q->balance == 1);
    assert(r->balance == -1 || r->balance == 0 || r->balance == 1);

    DEBUG_PRINTF("LR %ld  P: %2d  Q: %2d  R: %2d\n",
        (intptr_t)p->data, p->balance, q->balance, r->balance);

    rotate(q, r);
    rotate(p, r);

    q->balance -= 1 + MAX(0, r->balance);
    p->balance += 1 - MIN(MIN(0, r->balance) - 1, r->balance + q->balance);
    // r->balance += MAX(0, p->balance) + MIN(0, q->balance);

    // p->balance = (p->left && p->right) ? -MIN(r->balance, 0) : 0;
    // q->balance = - MAX(r->balance, 0);
    r->balance = 0;

    *height_change = -1;

    ASSERT_BALANCE(p);
    ASSERT_BALANCE(q);
    ASSERT_BALANCE(r);
    return r;
}

/**
 * Rotate right about `p->right` then right about `p`.
 *
 *    p               r
 *   / \             / \
 *  A   q    =>    p     q
 *     / \        / \   / \
 *    r   D      A   B C   D
 *   / \
 *  B   C
 *
 */
ZixTreeNode*
rotate_right_left(ZixTreeNode* p, int* height_change)
{
    ZixTreeNode* const q = p->right;
    ZixTreeNode* const r = q->left;

    assert(p->balance == 2);
    assert(q->balance == -1);
    assert(r->balance == -1 || r->balance == 0 || r->balance == 1);

    DEBUG_PRINTF("RL %ld  P: %2d  Q: %2d  R: %2d\n",
        (intptr_t)p->data, p->balance, q->balance, r->balance);

    rotate(q, r);
    rotate(p, r);

    q->balance += 1 - MIN(0, r->balance);
    p->balance -= 1 + MAX(MAX(0, r->balance) + 1, r->balance + q->balance);
    // r->balance += MAX(0, q->balance) + MIN(0, p->balance);

    // p->balance = (p->left && p->right) ? -MAX(r->balance, 0) : 0;
    // q->balance = - MIN(r->balance, 0);
    r->balance = 0;
    // assert(r->balance == 0);

    *height_change = -1;

    ASSERT_BALANCE(p);
    ASSERT_BALANCE(q);
    ASSERT_BALANCE(r);
    return r;
}

ZixTreeNode*
zix_tree_rebalance(ZixTree* t, ZixTreeNode* node, int* height_change)
{
#ifdef ZIX_TREE_HYPER_VERIFY
    const size_t old_height = height(node);
#endif
    DEBUG_PRINTF("REBALANCE %ld (%d)\n", (intptr_t)node->data, node->balance);
    *height_change = 0;
    const bool is_root = !node->parent;
    assert((is_root && t->root == node) || (!is_root && t->root != node));
    ZixTreeNode* replacement = node;
    if (node->balance == -2) {
        assert(node->left);
        if (node->left->balance == 1) {
            replacement = rotate_left_right(node, height_change);
        }
        else {
            replacement = rotate_right(node, height_change);
        }
    }
    else if (node->balance == 2) {
        assert(node->right);
        if (node->right->balance == -1) {
            replacement = rotate_right_left(node, height_change);
        }
        else {
            replacement = rotate_left(node, height_change);
        }
    }
    if (is_root) {
        assert(!replacement->parent);
        t->root = replacement;
    }
    DUMP(t);
#ifdef ZIX_TREE_HYPER_VERIFY
    assert(old_height + *height_change == height(replacement));
#endif
    return replacement;
}

ZixStatus
zix_tree_insert(ZixTree* t, void* e, ZixTreeIter** ti)
{
    DEBUG_PRINTF("**** INSERT %ld\n", (intptr_t)e);
    int          cmp = 0;
    ZixTreeNode* n = t->root;
    ZixTreeNode* p = NULL;

    // Find the parent p of e
    while (n) {
        p = n;
        cmp = t->cmp(e, n->data, t->cmp_data);
        if (cmp < 0) {
            n = n->left;
        }
        else if (cmp > 0) {
            n = n->right;
        }
        else if (t->allow_duplicates) {
            n = n->right;
        }
        else {
            if (ti) {
                *ti = n;
            }
            DEBUG_PRINTF("%ld EXISTS!\n", (intptr_t)e);
            return ZIX_STATUS_EXISTS;
        }
    }

    // Allocate a new node n
    if (!(n = (ZixTreeNode*)malloc(sizeof(ZixTreeNode)))) {
        return ZIX_STATUS_NO_MEM;
    }
    memset(n, '\0', sizeof(ZixTreeNode));
    n->data = e;
    n->balance = 0;
    if (ti) {
        *ti = n;
    }

    bool p_height_increased = false;

    // Make p the parent of n
    n->parent = p;
    if (!p) {
        t->root = n;
    }
    else {
        if (cmp < 0) {
            assert(!p->left);
            assert(p->balance == 0 || p->balance == 1);
            p->left = n;
            --p->balance;
            p_height_increased = !p->right;
        }
        else {
            assert(!p->right);
            assert(p->balance == 0 || p->balance == -1);
            p->right = n;
            ++p->balance;
            p_height_increased = !p->left;
        }
    }

    DUMP(t);

    // Rebalance if necessary (at most 1 rotation)
    assert(!p || p->balance == -1 || p->balance == 0 || p->balance == 1);
    if (p && p_height_increased) {
        int height_change = 0;
        for (ZixTreeNode* i = p; i && i->parent; i = i->parent) {
            if (i == i->parent->left) {
                if (--i->parent->balance == -2) {
                    zix_tree_rebalance(t, i->parent, &height_change);
                    break;
                }
            }
            else {
                assert(i == i->parent->right);
                if (++i->parent->balance == 2) {
                    zix_tree_rebalance(t, i->parent, &height_change);
                    break;
                }
            }

            if (i->parent->balance == 0) {
                break;
            }
        }
    }

    DUMP(t);

    ++t->size;

#ifdef ZIX_TREE_VERIFY
    if (!verify(t, t->root)) {
        return ZIX_STATUS_ERROR;
    }
#endif

    return ZIX_STATUS_SUCCESS;
}

ZixStatus
zix_tree_remove(ZixTree* t, ZixTreeIter* ti)
{
    ZixTreeNode* const n = ti;
    ZixTreeNode** pp = NULL;  // parent pointer
    ZixTreeNode* to_balance = n->parent;  // lowest node to balance
    int8_t             d_balance = 0;  // delta(balance) for n->parent

    DEBUG_PRINTF("*** REMOVE %ld\n", (intptr_t)n->data);

    if ((n == t->root) && !n->left && !n->right) {
        t->root = NULL;
        if (t->destroy) {
            t->destroy(n->data);
        }
        free(n);
        --t->size;
        assert(t->size == 0);
        return ZIX_STATUS_SUCCESS;
    }

    // Set pp to the parent pointer to n, if applicable
    if (n->parent) {
        assert(n->parent->left == n || n->parent->right == n);
        if (n->parent->left == n) {  // n is left child
            pp = &n->parent->left;
            d_balance = 1;
        }
        else {  // n is right child
            assert(n->parent->right == n);
            pp = &n->parent->right;
            d_balance = -1;
        }
    }

    assert(!pp || *pp == n);

    int height_change = 0;
    if (!n->left && !n->right) {
        // n is a leaf, just remove it
        if (pp) {
            *pp = NULL;
            to_balance = n->parent;
            height_change = (!n->parent->left && !n->parent->right) ? -1 : 0;
        }
    }
    else if (!n->left) {
        // Replace n with right (only) child
        if (pp) {
            *pp = n->right;
            to_balance = n->parent;
        }
        else {
            t->root = n->right;
        }
        n->right->parent = n->parent;
        height_change = -1;
    }
    else if (!n->right) {
        // Replace n with left (only) child
        if (pp) {
            *pp = n->left;
            to_balance = n->parent;
        }
        else {
            t->root = n->left;
        }
        n->left->parent = n->parent;
        height_change = -1;
    }
    else {
        // Replace n with in-order successor (leftmost child of right subtree)
        ZixTreeNode* replace = n->right;
        while (replace->left) {
            assert(replace->left->parent == replace);
            replace = replace->left;
        }

        // Remove replace from parent (replace_p)
        if (replace->parent->left == replace) {
            height_change = replace->parent->right ? 0 : -1;
            d_balance = 1;
            to_balance = replace->parent;
            replace->parent->left = replace->right;
        }
        else {
            assert(replace->parent == n);
            height_change = replace->parent->left ? 0 : -1;
            d_balance = -1;
            to_balance = replace->parent;
            replace->parent->right = replace->right;
        }

        if (to_balance == n) {
            to_balance = replace;
        }

        if (replace->right) {
            replace->right->parent = replace->parent;
        }

        replace->balance = n->balance;

        // Swap node to delete with replace
        if (pp) {
            *pp = replace;
        }
        else {
            assert(t->root == n);
            t->root = replace;
        }
        replace->parent = n->parent;
        replace->left = n->left;
        n->left->parent = replace;
        replace->right = n->right;
        if (n->right) {
            n->right->parent = replace;
        }

        assert(!replace->parent
            || replace->parent->left == replace
            || replace->parent->right == replace);
    }

    // Rebalance starting at to_balance upwards.
    for (ZixTreeNode* i = to_balance; i; i = i->parent) {
        i->balance += d_balance;
        if (d_balance == 0 || i->balance == -1 || i->balance == 1) {
            break;
        }

        assert(i != n);
        i = zix_tree_rebalance(t, i, &height_change);
        if (i->balance == 0) {
            height_change = -1;
        }

        if (i->parent) {
            if (i == i->parent->left) {
                d_balance = height_change * -1;
            }
            else {
                assert(i == i->parent->right);
                d_balance = height_change;
            }
        }
    }

    DUMP(t);

    if (t->destroy) {
        t->destroy(n->data);
    }
    free(n);

    --t->size;

#ifdef ZIX_TREE_VERIFY
    if (!verify(t, t->root)) {
        return ZIX_STATUS_ERROR;
    }
#endif

    return ZIX_STATUS_SUCCESS;
}

ZixStatus
zix_tree_find(const ZixTree* t, const void* e, ZixTreeIter** ti)
{
    ZixTreeNode* n = t->root;
    while (n) {
        const int cmp = t->cmp(e, n->data, t->cmp_data);
        if (cmp == 0) {
            break;
        }
        else if (cmp < 0) {
            n = n->left;
        }
        else {
            n = n->right;
        }
    }

    *ti = n;
    return (n) ? ZIX_STATUS_SUCCESS : ZIX_STATUS_NOT_FOUND;
}

void*
zix_tree_get(const ZixTreeIter* ti)
{
    return ti ? ti->data : NULL;
}

ZixTreeIter*
zix_tree_begin(ZixTree* t)
{
    if (!t->root) {
        return NULL;
    }

    ZixTreeNode* n = t->root;
    while (n->left) {
        n = n->left;
    }
    return n;
}

ZixTreeIter*
zix_tree_end(ZixTree* t)
{
    return NULL;
}

ZixTreeIter*
zix_tree_rbegin(ZixTree* t)
{
    if (!t->root) {
        return NULL;
    }

    ZixTreeNode* n = t->root;
    while (n->right) {
        n = n->right;
    }
    return n;
}

ZixTreeIter*
zix_tree_rend(ZixTree* t)
{
    return NULL;
}

bool
zix_tree_iter_is_end(const ZixTreeIter* i)
{
    return !i;
}

bool
zix_tree_iter_is_rend(const ZixTreeIter* i)
{
    return !i;
}

ZixTreeIter*
zix_tree_iter_next(ZixTreeIter* i)
{
    if (!i) {
        return NULL;
    }

    if (i->right) {
        i = i->right;
        while (i->left) {
            i = i->left;
        }
    }
    else {
        while (i->parent && i->parent->right == i) {  // i is a right child
            i = i->parent;
        }

        i = i->parent;
    }

    return i;
}

ZixTreeIter*
zix_tree_iter_prev(ZixTreeIter* i)
{
    if (!i) {
        return NULL;
    }

    if (i->left) {
        i = i->left;
        while (i->right) {
            i = i->right;
        }
    }
    else {
        while (i->parent && i->parent->left == i) {  // i is a left child
            i = i->parent;
        }

        i = i->parent;
    }

    return i;
}























#ifdef _WIN32
#ifndef _WIN32_WINNT
#    define _WIN32_WINNT 0x0600  /* for CreateSymbolicLink */
#endif
#    include <windows.h>
#    include <direct.h>
#    include <io.h>
#    define F_OK 0
#    define mkdir(path, flags) _mkdir(path)
#    if (defined(_MSC_VER) && _MSC_VER <= 1400) || defined(__MINGW64__) || defined(__MINGW32__)
/** Implement 'CreateSymbolicLink()' for MSVC 8 or earlier */
#ifdef __cplusplus
extern "C"
#endif
static BOOLEAN WINAPI
CreateSymbolicLink(LPCTSTR linkpath, LPCTSTR targetpath, DWORD flags)
{
    typedef BOOLEAN(WINAPI* PFUNC)(LPCTSTR, LPCTSTR, DWORD);

    PFUNC pfn = (PFUNC)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")),
        "CreateSymbolicLinkA");
    return pfn ? pfn(linkpath, targetpath, flags) : 0;
}
#    endif
#else
#    include <dirent.h>
#    include <unistd.h>
#endif

#if defined(HAVE_FLOCK) && defined(HAVE_FILENO)
#    include <sys/file.h>
#endif












#ifndef PAGE_SIZE
#    define PAGE_SIZE 4096
#endif

void
lilv_free(void* ptr)
{
    free(ptr);
}

char*
lilv_strjoin(const char* first, ...)
{
    size_t  len = strlen(first);
    char* result = (char*)malloc(len + 1);

    memcpy(result, first, len);

    va_list args;
    va_start(args, first);
    while (1) {
        const char* const s = va_arg(args, const char*);
        if (s == NULL) {
            break;
        }

        const size_t this_len = strlen(s);
        char* new_result = (char*)realloc(result, len + this_len + 1);
        if (!new_result) {
            free(result);
            return NULL;
        }

        result = new_result;
        memcpy(result + len, s, this_len);
        len += this_len;
    }
    va_end(args);

    result[len] = '\0';

    return result;
}

char*
lilv_strdup(const char* str)
{
    if (!str) {
        return NULL;
    }

    const size_t len = strlen(str);
    char* copy = (char*)malloc(len + 1);
    memcpy(copy, str, len + 1);
    return copy;
}

const char*
lilv_uri_to_path(const char* uri)
{
    return (const char*)serd_uri_to_path((const uint8_t*)uri);
}

char*
lilv_file_uri_parse(const char* uri, char** hostname)
{
    return (char*)serd_file_uri_parse((const uint8_t*)uri, (uint8_t**)hostname);
}

/** Return the current LANG converted to Turtle (i.e. RFC3066) style.
 * For example, if LANG is set to "en_CA.utf-8", this returns "en-ca".
 */
char*
lilv_get_lang(void)
{
    const char* const env_lang = getenv("LANG");
    if (!env_lang || !strcmp(env_lang, "")
        || !strcmp(env_lang, "C") || !strcmp(env_lang, "POSIX")) {
        return NULL;
    }

    const size_t env_lang_len = strlen(env_lang);
    char* const  lang = (char*)malloc(env_lang_len + 1);
    for (size_t i = 0; i < env_lang_len + 1; ++i) {
        if (env_lang[i] == '_') {
            lang[i] = '-';  // Convert _ to -
        }
        else if (env_lang[i] >= 'A' && env_lang[i] <= 'Z') {
            lang[i] = env_lang[i] + ('a' - 'A');  // Convert to lowercase
        }
        else if (env_lang[i] >= 'a' && env_lang[i] <= 'z') {
            lang[i] = env_lang[i];  // Lowercase letter, copy verbatim
        }
        else if (env_lang[i] >= '0' && env_lang[i] <= '9') {
            lang[i] = env_lang[i];  // Digit, copy verbatim
        }
        else if (env_lang[i] == '\0' || env_lang[i] == '.') {
            // End, or start of suffix (e.g. en_CA.utf-8), finished
            lang[i] = '\0';
            break;
        }
        else {
            LILV_ERRORF("Illegal LANG `%s' ignored\n", env_lang);
            free(lang);
            return NULL;
        }
    }

    return lang;
}

#ifndef _WIN32

/** Append suffix to dst, update dst_len, and return the realloc'd result. */
static char*
strappend(char* dst, size_t* dst_len, const char* suffix, size_t suffix_len)
{
    dst = (char*)realloc(dst, *dst_len + suffix_len + 1);
    memcpy(dst + *dst_len, suffix, suffix_len);
    dst[(*dst_len += suffix_len)] = '\0';
    return dst;
}

/** Append the value of the environment variable var to dst. */
static char*
append_var(char* dst, size_t* dst_len, const char* var)
{
    // Get value from environment
    const char* val = getenv(var);
    if (val) {  // Value found, append it
        return strappend(dst, dst_len, val, strlen(val));
    }
    else {  // No value found, append variable reference as-is
        return strappend(strappend(dst, dst_len, "$", 1),
            dst_len, var, strlen(var));
    }
}

#endif

/** Expand variables (e.g. POSIX ~ or $FOO, Windows %FOO%) in `path`. */
char*
lilv_expand(const char* path)
{
#ifdef _WIN32
    char* out = (char*)malloc(MAX_PATH);
    ExpandEnvironmentStrings(path, out, MAX_PATH);
#else
    char* out = NULL;
    size_t len = 0;

    const char* start = path;  // Start of current chunk to copy
    for (const char* s = path; *s;) {
        if (*s == '$') {
            // Hit $ (variable reference, e.g. $VAR_NAME)
            for (const char* t = s + 1; ; ++t) {
                if (!*t || (!isupper(*t) && !isdigit(*t) && *t != '_')) {
                    // Append preceding chunk
                    out = strappend(out, &len, start, s - start);

                    // Append variable value (or $VAR_NAME if not found)
                    char* var = (char*)calloc(t - s, 1);
                    memcpy(var, s + 1, t - s - 1);
                    out = append_var(out, &len, var);
                    free(var);

                    // Continue after variable reference
                    start = s = t;
                    break;
                }
            }
        }
        else if (*s == '~' && (*(s + 1) == '/' || !*(s + 1))) {
            // Hit ~ before slash or end of string (home directory reference)
            out = strappend(out, &len, start, s - start);
            out = append_var(out, &len, "HOME");
            start = ++s;
        }
        else {
            ++s;
        }
    }

    if (*start) {
        out = strappend(out, &len, start, strlen(start));
    }
#endif

    return out;
}

static bool
lilv_is_dir_sep(const char c)
{
    return c == '/' || c == LILV_DIR_SEP[0];
}

char*
lilv_dirname(const char* path)
{
    const char* s = path + strlen(path) - 1;  // Last character
    for (; s > path && lilv_is_dir_sep(*s); --s) {}  // Last non-slash
    for (; s > path && !lilv_is_dir_sep(*s); --s) {}  // Last internal slash
    for (; s > path && lilv_is_dir_sep(*s); --s) {}  // Skip duplicates

    if (s == path) {  // Hit beginning
        return lilv_is_dir_sep(*s) ? lilv_strdup("/") : lilv_strdup(".");
    }
    else {  // Pointing to the last character of the result (inclusive)
        char* dirname = (char*)malloc(s - path + 2);
        memcpy(dirname, path, s - path + 1);
        dirname[s - path + 1] = '\0';
        return dirname;
    }
}

char*
lilv_dir_path(const char* path)
{
    if (!path) {
        return NULL;
    }

    const size_t len = strlen(path);

    if (lilv_is_dir_sep(path[len - 1])) {
        return lilv_strdup(path);
    }

    char* dir_path = (char*)calloc(len + 2, 1);
    memcpy(dir_path, path, len);
    dir_path[len] = LILV_DIR_SEP[0];
    return dir_path;
}

bool
lilv_path_exists(const char* path, const void* ignored)
{
#ifdef HAVE_LSTAT
    struct stat st;
    return !lstat(path, &st);
#else
    return !access(path, F_OK);
#endif
}

char*
lilv_find_free_path(const char* in_path,
    bool (*exists)(const char*, const void*),
    const void* user_data)
{
    const size_t in_path_len = strlen(in_path);
    char* path = (char*)malloc(in_path_len + 7);
    memcpy(path, in_path, in_path_len + 1);

    for (int i = 2; i < 1000000; ++i) {
        if (!exists(path, user_data)) {
            return path;
        }
        snprintf(path, in_path_len + 7, "%s.%u", in_path, i);
    }

    return NULL;
}

int
lilv_copy_file(const char* src, const char* dst)
{
    FILE* in = fopen(src, "r");
    if (!in) {
        return errno;
    }

    FILE* out = fopen(dst, "w");
    if (!out) {
        fclose(in);
        return errno;
    }

    char* page = (char*)malloc(PAGE_SIZE);
    size_t n_read = 0;
    int    st = 0;
    while ((n_read = fread(page, 1, PAGE_SIZE, in)) > 0) {
        if (fwrite(page, 1, n_read, out) != n_read) {
            st = errno;
            break;
        }
    }

    if (!st && (ferror(in) || ferror(out))) {
        st = EBADF;
    }

    free(page);
    fclose(in);
    fclose(out);

    return st;
}

static inline bool
is_windows_path(const char* path)
{
    return (isalpha(path[0]) && (path[1] == ':' || path[1] == '|') &&
        (path[2] == '/' || path[2] == '\\'));
}

bool
lilv_path_is_absolute(const char* path)
{
    if (lilv_is_dir_sep(path[0])) {
        return true;
    }

#ifdef _WIN32
    if (is_windows_path(path)) {
        return true;
    }
#endif

    return false;
}

char*
lilv_path_absolute(const char* path)
{
    if (lilv_path_is_absolute(path)) {
        return lilv_strdup(path);
    }
    else {
        char* cwd = getcwd(NULL, 0);
        char* abs_path = lilv_path_join(cwd, path);
        free(cwd);
        return abs_path;
    }
}

char*
lilv_path_join(const char* a, const char* b)
{
    if (!a) {
        return lilv_strdup(b);
    }

    const size_t a_len = strlen(a);
    const size_t b_len = b ? strlen(b) : 0;
    const size_t pre_len = a_len - (lilv_is_dir_sep(a[a_len - 1]) ? 1 : 0);
    char* path = (char*)calloc(1, a_len + b_len + 2);
    memcpy(path, a, pre_len);
    path[pre_len] = '/';
    if (b) {
        memcpy(path + pre_len + 1,
            b + (lilv_is_dir_sep(b[0]) ? 1 : 0),
            lilv_is_dir_sep(b[0]) ? b_len - 1 : b_len);
    }
    return path;
}

typedef struct {
    char* pattern;
    time_t time;
    char* latest;
} Latest;

static void
update_latest(const char* path, const char* name, void* data)
{
    Latest* latest = (Latest*)data;
    char* entry_path = lilv_path_join(path, name);
    unsigned num;
    if (sscanf(entry_path, latest->pattern, &num) == 1) {
        struct stat st;
        if (!stat(entry_path, &st)) {
            if (st.st_mtime >= latest->time) {
                free(latest->latest);
                latest->latest = entry_path;
            }
        }
        else {
            LILV_ERRORF("stat(%s) (%s)\n", path, strerror(errno));
        }
    }
    if (entry_path != latest->latest) {
        free(entry_path);
    }
}

/** Return the latest copy of the file at `path` that is newer. */
char*
lilv_get_latest_copy(const char* path, const char* copy_path)
{
    char* copy_dir = lilv_dirname(copy_path);
    Latest latest = { lilv_strjoin(copy_path, ".%u", NULL), 0, NULL };

    struct stat st;
    if (!stat(path, &st)) {
        latest.time = st.st_mtime;
    }
    else {
        LILV_ERRORF("stat(%s) (%s)\n", path, strerror(errno));
    }

    lilv_dir_for_each(copy_dir, &latest, update_latest);

    free(latest.pattern);
    free(copy_dir);
    return latest.latest;
}

char*
lilv_realpath(const char* path)
{
    if (!path) {
        return NULL;
    }

#if defined(_WIN32)
    char* out = (char*)malloc(MAX_PATH);
    GetFullPathName(path, MAX_PATH, out, NULL);
    return out;
#else
    char* real_path = realpath(path, NULL);
    return real_path ? real_path : lilv_strdup(path);
#endif
}

int
lilv_symlink(const char* oldpath, const char* newpath)
{
    int ret = 0;
    if (strcmp(oldpath, newpath)) {
#ifdef _WIN32
        ret = !CreateSymbolicLink(newpath, oldpath, 0);
        if (ret) {
            ret = !CreateHardLink(newpath, oldpath, 0);
        }
#else
        ret = symlink(oldpath, newpath);
#endif
    }
    if (ret) {
        LILV_ERRORF("Failed to link %s => %s (%s)\n",
            newpath, oldpath, strerror(errno));
    }
    return ret;
}

char*
lilv_path_relative_to(const char* path, const char* base)
{
    const size_t path_len = strlen(path);
    const size_t base_len = strlen(base);
    const size_t min_len = (path_len < base_len) ? path_len : base_len;

    // Find the last separator common to both paths
    size_t last_shared_sep = 0;
    for (size_t i = 0; i < min_len && path[i] == base[i]; ++i) {
        if (lilv_is_dir_sep(path[i])) {
            last_shared_sep = i;
        }
    }

    if (last_shared_sep == 0) {
        // No common components, return path
        return lilv_strdup(path);
    }

    // Find the number of up references ("..") required
    size_t up = 0;
    for (size_t i = last_shared_sep + 1; i < base_len; ++i) {
        if (lilv_is_dir_sep(base[i])) {
            ++up;
        }
    }

    // Write up references
    const size_t suffix_len = path_len - last_shared_sep;
    char* rel = (char*)calloc(1, suffix_len + (up * 3) + 1);
    for (size_t i = 0; i < up; ++i) {
        memcpy(rel + (i * 3), "../", 3);
    }

    // Write suffix
    memcpy(rel + (up * 3), path + last_shared_sep + 1, suffix_len);
    return rel;
}

bool
lilv_path_is_child(const char* path, const char* dir)
{
    if (path && dir) {
        const size_t path_len = strlen(path);
        const size_t dir_len = strlen(dir);
        return dir && path_len >= dir_len && !strncmp(path, dir, dir_len);
    }
    return false;
}

int
lilv_flock(FILE* file, bool lock)
{
#if defined(HAVE_FLOCK) && defined(HAVE_FILENO)
    return flock(fileno(file), lock ? LOCK_EX : LOCK_UN);
#else
    return 0;
#endif
}

void
lilv_dir_for_each(const char* path,
    void* data,
    void (*f)(const char* path, const char* name, void* data))
{
#ifdef _WIN32
    char* pat = lilv_path_join(path, "*");
    WIN32_FIND_DATA fd;
    HANDLE          fh = FindFirstFile(pat, &fd);
    if (fh != INVALID_HANDLE_VALUE) {
        do {
            f(path, fd.cFileName, data);
        } while (FindNextFile(fh, &fd));
    }
    free(pat);
#else
    DIR* dir = opendir(path);
    if (dir) {
        for (struct dirent* entry; (entry = readdir(dir));) {
            f(path, entry->d_name, data);
        }
        closedir(dir);
    }
#endif
}

int
lilv_mkdir_p(const char* dir_path)
{
    char* path = lilv_strdup(dir_path);
    const size_t path_len = strlen(path);
    size_t       i = 1;

#ifdef _WIN32
    if (is_windows_path(dir_path)) {
        i = 3;
    }
#endif

    for (; i <= path_len; ++i) {
        const char c = path[i];
        if (c == LILV_DIR_SEP[0] || c == '/' || c == '\0') {
            path[i] = '\0';
            if (mkdir(path, 0755) && errno != EEXIST) {
                free(path);
                return errno;
            }
            path[i] = c;
        }
    }

    free(path);
    return 0;
}

static off_t
lilv_file_size(const char* path)
{
    struct stat buf;
    if (stat(path, &buf)) {
        LILV_ERRORF("stat(%s) (%s)\n", path, strerror(errno));
        return 0;
    }
    return buf.st_size;
}

bool
lilv_file_equals(const char* a_path, const char* b_path)
{
    if (!strcmp(a_path, b_path)) {
        return true;  // Paths match
    }

    bool        match = false;
    FILE* a_file = NULL;
    FILE* b_file = NULL;
    char* const a_real = lilv_realpath(a_path);
    char* const b_real = lilv_realpath(b_path);
    if (!strcmp(a_real, b_real)) {
        match = true;  // Real paths match
    }
    else if (lilv_file_size(a_path) != lilv_file_size(b_path)) {
        match = false;  // Sizes differ
    }
    else if (!(a_file = fopen(a_real, "rb")) ||
        !(b_file = fopen(b_real, "rb"))) {
        match = false;  // Missing file matches nothing
    }
    else {
        // TODO: Improve performance by reading chunks
        match = true;
        while (!feof(a_file) && !feof(b_file)) {
            if (fgetc(a_file) != fgetc(b_file)) {
                match = false;
                break;
            }
        }
    }

    if (a_file) {
        fclose(a_file);
    }
    if (b_file) {
        fclose(b_file);
    }
    free(a_real);
    free(b_real);
    return match;
}


/**********************SERD********************/
/*
  Copyright 2011-2016 David Robillard <http://drobilla.net>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "lilv.h"
#include <math.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <float.h>


#define _POSIX_C_SOURCE 200809L /* for posix_memalign and posix_fadvise */



//#include "serd_config.h"

#if defined(HAVE_POSIX_FADVISE) && defined(HAVE_FILENO)
#   include <fcntl.h>
#endif

#define NS_XSD "http://www.w3.org/2001/XMLSchema#"
#define NS_RDF "http://www.w3.org/1999/02/22-rdf-syntax-ns#"

#define SERD_PAGE_SIZE 4096

#ifndef MIN
#    define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#if defined(__GNUC__)
#    define SERD_LOG_FUNC(fmt, arg1) __attribute__((format(printf, fmt, arg1)))
#else
#    define SERD_LOG_FUNC(fmt, arg1)
#endif

static const uint8_t replacement_char[] = { 0xEF, 0xBF, 0xBD };

/* File and Buffer Utilities */

static inline FILE*
serd_fopen(const char* path, const char* mode)
{
    FILE* fd = fopen(path, mode);
    if (!fd) {
        fprintf(stderr, "error: failed to open file %s (%s)\n",
            path, strerror(errno));
        return NULL;
    }
#if defined(HAVE_POSIX_FADVISE) && defined(HAVE_FILENO)
    posix_fadvise(fileno(fd), 0, 0, POSIX_FADV_SEQUENTIAL);
#endif
    return fd;
}

static inline void*
serd_bufalloc(size_t size)
{
#ifdef HAVE_POSIX_MEMALIGN
    void* ptr;
    const int ret = posix_memalign(&ptr, SERD_PAGE_SIZE, size);
    return ret ? NULL : ptr;
#else
    return malloc(size);
#endif
}

/* Byte source */

typedef struct {
    const uint8_t* filename;
    unsigned       line;
    unsigned       col;
} Cursor;

typedef struct {
    SerdSource          read_func;    ///< Read function (e.g. fread)
    SerdStreamErrorFunc error_func;   ///< Error function (e.g. ferror)
    void* stream;       ///< Stream (e.g. FILE)
    size_t              page_size;    ///< Number of bytes to read at a time
    size_t              buf_size;     ///< Number of bytes in file_buf
    Cursor              cur;          ///< Cursor for error reporting
    uint8_t* file_buf;     ///< Buffer iff reading pages from a file
    const uint8_t* read_buf;     ///< Pointer to file_buf or read_byte
    size_t              read_head;    ///< Offset into read_buf
    uint8_t             read_byte;    ///< 1-byte 'buffer' used when not paging
    bool                from_stream;  ///< True iff reading from `stream`
    bool                prepared;     ///< True iff prepared for reading
    bool                eof;          ///< True iff end of file reached
} SerdByteSource;

SerdStatus
serd_byte_source_open_file(SerdByteSource* source,
    FILE* file,
    bool            bulk);

SerdStatus
serd_byte_source_open_string(SerdByteSource* source, const uint8_t* utf8);

SerdStatus
serd_byte_source_open_source(SerdByteSource* source,
    SerdSource          read_func,
    SerdStreamErrorFunc error_func,
    void* stream,
    const uint8_t* name,
    size_t              page_size);

SerdStatus
serd_byte_source_close(SerdByteSource* source);

SerdStatus
serd_byte_source_prepare(SerdByteSource* source);

SerdStatus
serd_byte_source_page(SerdByteSource* source);

static inline uint8_t
serd_byte_source_peek(SerdByteSource* source)
{
    assert(source->prepared);
    return source->read_buf[source->read_head];
}

static inline SerdStatus
serd_byte_source_advance(SerdByteSource* source)
{
    SerdStatus st = SERD_SUCCESS;

    switch (serd_byte_source_peek(source)) {
    case '\n': ++source->cur.line; source->cur.col = 0; break;
    default:   ++source->cur.col;
    }

    const bool was_eof = source->eof;
    if (source->from_stream) {
        source->eof = false;
        if (source->page_size > 1) {
            if (++source->read_head == source->page_size) {
                st = serd_byte_source_page(source);
            }
            else if (source->read_head == source->buf_size) {
                source->eof = true;
            }
        }
        else {
            if (!source->read_func(&source->read_byte, 1, 1, source->stream)) {
                source->eof = true;
                st = source->error_func(source->stream) ? SERD_ERR_UNKNOWN
                    : SERD_FAILURE;
            }
        }
    }
    else if (!source->eof) {
        ++source->read_head; // Move to next character in string
        if (source->read_buf[source->read_head] == '\0') {
            source->eof = true;
        }
    }

    return (was_eof && source->eof) ? SERD_FAILURE : st;
}

/* Stack */

/** A dynamic stack in memory. */
typedef struct {
    uint8_t* buf;       ///< Stack memory
    size_t   buf_size;  ///< Allocated size of buf (>= size)
    size_t   size;      ///< Conceptual size of stack in buf
} SerdStack;

/** An offset to start the stack at. Note 0 is reserved for NULL. */
#define SERD_STACK_BOTTOM sizeof(void*)

static inline SerdStack
serd_stack_new(size_t size)
{
    SerdStack stack;
    stack.buf = (uint8_t*)calloc(size, 1);
    stack.buf_size = size;
    stack.size = SERD_STACK_BOTTOM;
    return stack;
}

static inline bool
serd_stack_is_empty(SerdStack* stack)
{
    return stack->size <= SERD_STACK_BOTTOM;
}

static inline void
serd_stack_free(SerdStack* stack)
{
    free(stack->buf);
    stack->buf = NULL;
    stack->buf_size = 0;
    stack->size = 0;
}

static inline uint8_t*
serd_stack_push(SerdStack* stack, size_t n_bytes)
{
    const size_t new_size = stack->size + n_bytes;
    if (stack->buf_size < new_size) {
        stack->buf_size += (stack->buf_size >> 1); // *= 1.5
        stack->buf = (uint8_t*)realloc(stack->buf, stack->buf_size);
    }
    uint8_t* const ret = (stack->buf + stack->size);
    stack->size = new_size;
    return ret;
}

static inline void
serd_stack_pop(SerdStack* stack, size_t n_bytes)
{
    assert(stack->size >= n_bytes);
    stack->size -= n_bytes;
}

static inline void*
serd_stack_push_aligned(SerdStack* stack, size_t n_bytes, size_t align)
{
    // Push one byte to ensure space for a pad count
    serd_stack_push(stack, 1);

    // Push padding if necessary
    const size_t pad = align - stack->size % align;
    if (pad > 0) {
        serd_stack_push(stack, pad);
    }

    // Set top of stack to pad count so we can properly pop later
    assert(pad < UINT8_MAX);
    stack->buf[stack->size - 1] = (uint8_t)pad;

    // Push requested space at aligned location
    return serd_stack_push(stack, n_bytes);
}

static inline void
serd_stack_pop_aligned(SerdStack* stack, size_t n_bytes)
{
    // Pop requested space down to aligned location
    serd_stack_pop(stack, n_bytes);

    // Get amount of padding from top of stack
    const uint8_t pad = stack->buf[stack->size - 1];

    // Pop padding and pad count
    serd_stack_pop(stack, pad + 1u);
}

/* Byte Sink */

typedef struct SerdByteSinkImpl {
    SerdSink sink;
    void* stream;
    uint8_t* buf;
    size_t   size;
    size_t   block_size;
} SerdByteSink;

static inline SerdByteSink
serd_byte_sink_new(SerdSink sink, void* stream, size_t block_size)
{
    SerdByteSink bsink;
    bsink.sink = sink;
    bsink.stream = stream;
    bsink.size = 0;
    bsink.block_size = block_size;
    bsink.buf = ((block_size > 1)
        ? (uint8_t*)serd_bufalloc(block_size)
        : NULL);
    return bsink;
}

static inline void
serd_byte_sink_flush(SerdByteSink* bsink)
{
    if (bsink->block_size > 1 && bsink->size > 0) {
        bsink->sink(bsink->buf, bsink->size, bsink->stream);
        bsink->size = 0;
    }
}

static inline void
serd_byte_sink_free(SerdByteSink* bsink)
{
    serd_byte_sink_flush(bsink);
    free(bsink->buf);
    bsink->buf = NULL;
}

static inline size_t
serd_byte_sink_write(const void* buf, size_t len, SerdByteSink* bsink)
{
    if (len == 0) {
        return 0;
    }
    else if (bsink->block_size == 1) {
        return bsink->sink(buf, len, bsink->stream);
    }

    const size_t orig_len = len;
    while (len) {
        const size_t space = bsink->block_size - bsink->size;
        const size_t n = MIN(space, len);

        // Write as much as possible into the remaining buffer space
        memcpy(bsink->buf + bsink->size, buf, n);
        bsink->size += n;
        buf = (const uint8_t*)buf + n;
        len -= n;

        // Flush page if buffer is full
        if (bsink->size == bsink->block_size) {
            bsink->sink(bsink->buf, bsink->block_size, bsink->stream);
            bsink->size = 0;
        }
    }
    return orig_len;
}

/* Character utilities */

/** Return true if `c` lies within [`min`...`max`] (inclusive) */
static inline bool
in_range(const int c, const int min, const int max)
{
    return (c >= min && c <= max);
}

/** RFC2234: ALPHA ::= %x41-5A / %x61-7A  ; A-Z / a-z */
static inline bool
is_alpha(const int c)
{
    return in_range(c, 'A', 'Z') || in_range(c, 'a', 'z');
}

/** RFC2234: DIGIT ::= %x30-39  ; 0-9 */
static inline bool
is_digit(const int c)
{
    return in_range(c, '0', '9');
}

/* RFC2234: HEXDIG ::= DIGIT / "A" / "B" / "C" / "D" / "E" / "F" */
static inline bool
is_hexdig(const int c)
{
    return is_digit(c) || in_range(c, 'A', 'F');
}

/* Turtle / JSON / C: XDIGIT ::= DIGIT / A-F / a-f */
static inline bool
is_xdigit(const int c)
{
    return is_hexdig(c) || in_range(c, 'a', 'f');
}

static inline bool
is_space(const char c)
{
    switch (c) {
    case ' ': case '\f': case '\n': case '\r': case '\t': case '\v':
        return true;
    default:
        return false;
    }
}

static inline bool
is_base64(const uint8_t c)
{
    return is_alpha(c) || is_digit(c) || c == '+' || c == '/' || c == '=';
}


/* String utilities */

size_t
serd_substrlen(const uint8_t* str,
    size_t         len,
    size_t* n_bytes,
    SerdNodeFlags* flags);

static inline int
serd_strncasecmp(const char* s1, const char* s2, size_t n)
{
    for (; n > 0 && *s2; s1++, s2++, --n) {
        if (toupper(*s1) != toupper(*s2)) {
            return ((*(const uint8_t*)s1 < *(const uint8_t*)s2) ? -1 : +1);
        }
    }
    return 0;
}

static inline uint32_t
utf8_num_bytes(const uint8_t c)
{
    if ((c & 0x80) == 0) {  // Starts with `0'
        return 1;
    }
    else if ((c & 0xE0) == 0xC0) {  // Starts with `110'
        return 2;
    }
    else if ((c & 0xF0) == 0xE0) {  // Starts with `1110'
        return 3;
    }
    else if ((c & 0xF8) == 0xF0) {  // Starts with `11110'
        return 4;
    }
    return 0;
}

/// Return the code point of a UTF-8 character with known length
static inline uint32_t
parse_counted_utf8_char(const uint8_t* utf8, size_t size)
{
    uint32_t c = utf8[0] & ((1u << (8 - size)) - 1);
    for (size_t i = 1; i < size; ++i) {
        const uint8_t in = utf8[i] & 0x3F;
        c = (c << 6) | in;
    }
    return c;
}

/// Parse a UTF-8 character, set *size to the length, and return the code point
static inline uint32_t
parse_utf8_char(const uint8_t* utf8, size_t* size)
{
    switch (*size = utf8_num_bytes(utf8[0])) {
    case 1: case 2: case 3: case 4:
        return parse_counted_utf8_char(utf8, *size);
    default:
        *size = 0;
        return 0;
    }
}

/* URI utilities */

static inline bool
chunk_equals(const SerdChunk* a, const SerdChunk* b)
{
    return a->len == b->len
        && !strncmp((const char*)a->buf, (const char*)b->buf, a->len);
}

static inline size_t
uri_path_len(const SerdURI* uri)
{
    return uri->path_base.len + uri->path.len;
}

static inline uint8_t
uri_path_at(const SerdURI* uri, size_t i)
{
    if (i < uri->path_base.len) {
        return uri->path_base.buf[i];
    }
    else {
        return uri->path.buf[i - uri->path_base.len];
    }
}

/**
   Return the index of the first differing character after the last root slash,
   or zero if `uri` is not under `root`.
*/
static inline size_t
uri_rooted_index(const SerdURI* uri, const SerdURI* root)
{
    if (!root || !root->scheme.len ||
        !chunk_equals(&root->scheme, &uri->scheme) ||
        !chunk_equals(&root->authority, &uri->authority)) {
        return 0;
    }

    bool         differ = false;
    const size_t path_len = uri_path_len(uri);
    const size_t root_len = uri_path_len(root);
    size_t       last_root_slash = 0;
    for (size_t i = 0; i < path_len && i < root_len; ++i) {
        const uint8_t u = uri_path_at(uri, i);
        const uint8_t r = uri_path_at(root, i);

        differ = differ || u != r;
        if (r == '/') {
            last_root_slash = i;
            if (differ) {
                return 0;
            }
        }
    }

    return last_root_slash + 1;
}

/** Return true iff `uri` shares path components with `root` */
static inline bool
uri_is_related(const SerdURI* uri, const SerdURI* root)
{
    return uri_rooted_index(uri, root) > 0;
}

/** Return true iff `uri` is within the base of `root` */
static inline bool
uri_is_under(const SerdURI* uri, const SerdURI* root)
{
    const size_t index = uri_rooted_index(uri, root);
    return index > 0 && uri->path.len > index;
}

static inline bool
is_uri_scheme_char(const int c)
{
    switch (c) {
    case ':': case '+': case '-': case '.':
        return true;
    default:
        return is_alpha(c) || is_digit(c);
    }
}

/* Error reporting */

static inline void
serd_error(SerdErrorSink error_sink, void* handle, const SerdError* e)
{
    if (error_sink) {
        error_sink(handle, e);
    }
    else {
        fprintf(stderr, "error: %s:%u:%u: ", e->filename, e->line, e->col);
        vfprintf(stderr, e->fmt, *e->args);
    }
}

SERD_LOG_FUNC(3, 4)
int
r_err(SerdReader* reader, SerdStatus st, const char* fmt, ...);

/* Reader */

#ifdef SERD_STACK_CHECK
#    define SERD_STACK_ASSERT_TOP(reader, ref) \
            assert(ref == reader->allocs[reader->n_allocs - 1]);
#else
#    define SERD_STACK_ASSERT_TOP(reader, ref)
#endif

/* Reference to a node in the stack (we can not use pointers since the
   stack may be reallocated, invalidating any pointers to elements).
*/
typedef size_t Ref;

typedef struct {
    Ref                 graph;
    Ref                 subject;
    Ref                 predicate;
    Ref                 object;
    Ref                 datatype;
    Ref                 lang;
    SerdStatementFlags* flags;
} ReadContext;

struct SerdReaderImpl {
    void* handle;
    void              (*free_handle)(void* ptr);
    SerdBaseSink      base_sink;
    SerdPrefixSink    prefix_sink;
    SerdStatementSink statement_sink;
    SerdEndSink       end_sink;
    SerdErrorSink     error_sink;
    void* error_handle;
    Ref               rdf_first;
    Ref               rdf_rest;
    Ref               rdf_nil;
    SerdNode          default_graph;
    SerdByteSource    source;
    SerdStack         stack;
    SerdSyntax        syntax;
    unsigned          next_id;
    SerdStatus        status;
    uint8_t* buf;
    uint8_t* bprefix;
    size_t            bprefix_len;
    bool              strict;     ///< True iff strict parsing
    bool              seen_genid;
#ifdef SERD_STACK_CHECK
    Ref* allocs;     ///< Stack of push offsets
    size_t            n_allocs;   ///< Number of stack pushes
#endif
};

Ref push_node_padded(SerdReader* reader,
    size_t      maxlen,
    SerdType    type,
    const char* str,
    size_t      n_bytes);

Ref push_node(SerdReader* reader,
    SerdType    type,
    const char* str,
    size_t      n_bytes);

size_t genid_size(SerdReader* reader);
Ref    blank_id(SerdReader* reader);
void   set_blank_id(SerdReader* reader, Ref ref, size_t buf_size);

SerdNode* deref(SerdReader* reader, Ref ref);

Ref pop_node(SerdReader* reader, Ref ref);

bool emit_statement(SerdReader* reader, ReadContext ctx, Ref o, Ref d, Ref l);

bool read_n3_statement(SerdReader* reader);
bool read_nquadsDoc(SerdReader* reader);
bool read_turtleTrigDoc(SerdReader* reader);

typedef enum {
    FIELD_NONE,
    FIELD_SUBJECT,
    FIELD_PREDICATE,
    FIELD_OBJECT,
    FIELD_GRAPH
} Field;


static inline int
peek_byte(SerdReader* reader)
{
    SerdByteSource* source = &reader->source;

    return source->eof ? EOF : (int)source->read_buf[source->read_head];
}

static inline int
eat_byte(SerdReader* reader)
{
    const int        c = peek_byte(reader);
    const SerdStatus st = serd_byte_source_advance(&reader->source);
    if (st) {
        reader->status = st;
    }
    return c;
}

static inline int
eat_byte_safe(SerdReader* reader, const int byte)
{
    (void)byte;

    const int c = eat_byte(reader);
    assert(c == byte);
    return c;
}

static inline int
eat_byte_check(SerdReader* reader, const int byte)
{
    const int c = peek_byte(reader);
    if (c != byte) {
        return r_err(reader, SERD_ERR_BAD_SYNTAX,
            "expected `%c', not `%c'\n", byte, c);
    }
    return eat_byte_safe(reader, byte);
}

static inline bool
eat_string(SerdReader* reader, const char* str, unsigned n)
{
    bool bad = false;
    for (unsigned i = 0; i < n; ++i) {
        bad |= (bool)eat_byte_check(reader, ((const uint8_t*)str)[i]);
    }
    return bad;
}

static inline SerdStatus
push_byte(SerdReader* reader, Ref ref, const int c)
{
    assert(c != EOF);
    SERD_STACK_ASSERT_TOP(reader, ref);

    uint8_t* const  s = serd_stack_push(&reader->stack, 1);
    SerdNode* const node = (SerdNode*)(reader->stack.buf + ref);
    ++node->n_bytes;
    if (!(c & 0x80)) {  // Starts with 0 bit, start of new character
        ++node->n_chars;
    }
    *(s - 1) = (uint8_t)c;
    *s = '\0';
    return SERD_SUCCESS;
}

static inline void
push_bytes(SerdReader* reader, Ref ref, const uint8_t* bytes, unsigned len)
{
    for (unsigned i = 0; i < len; ++i) {
        push_byte(reader, ref, bytes[i]);
    }
}

/* Stack */


void
serd_free(void* ptr)
{
    free(ptr);
}

const uint8_t*
serd_strerror(SerdStatus status)
{
    switch (status) {
    case SERD_SUCCESS:        return (const uint8_t*)"Success";
    case SERD_FAILURE:        return (const uint8_t*)"Non-fatal failure";
    case SERD_ERR_UNKNOWN:    return (const uint8_t*)"Unknown error";
    case SERD_ERR_BAD_SYNTAX: return (const uint8_t*)"Invalid syntax";
    case SERD_ERR_BAD_ARG:    return (const uint8_t*)"Invalid argument";
    case SERD_ERR_NOT_FOUND:  return (const uint8_t*)"Not found";
    case SERD_ERR_ID_CLASH:   return (const uint8_t*)"Blank node ID clash";
    case SERD_ERR_BAD_CURIE:  return (const uint8_t*)"Invalid CURIE";
    case SERD_ERR_INTERNAL:   return (const uint8_t*)"Internal error";
    }
    return (const uint8_t*)"Unknown error";  // never reached
}

static inline void
serd_update_flags(const uint8_t c, SerdNodeFlags* const flags)
{
    switch (c) {
    case '\r': case '\n':
        *flags |= SERD_HAS_NEWLINE;
        break;
    case '"':
        *flags |= SERD_HAS_QUOTE;
    }
}

size_t
serd_substrlen(const uint8_t* const str,
    const size_t         len,
    size_t* const        n_bytes,
    SerdNodeFlags* const flags)
{
    size_t        n_chars = 0;
    size_t        i = 0;
    SerdNodeFlags f = 0;
    for (; i < len && str[i]; ++i) {
        if ((str[i] & 0xC0) != 0x80) {  // Start of new character
            ++n_chars;
            serd_update_flags(str[i], &f);
        }
    }
    if (n_bytes) {
        *n_bytes = i;
    }
    if (flags) {
        *flags = f;
    }
    return n_chars;
}

size_t
serd_strlen(const uint8_t* str, size_t* n_bytes, SerdNodeFlags* flags)
{
    size_t        n_chars = 0;
    size_t        i = 0;
    SerdNodeFlags f = 0;
    for (; str[i]; ++i) {
        if ((str[i] & 0xC0) != 0x80) {  // Start of new character
            ++n_chars;
            serd_update_flags(str[i], &f);
        }
    }
    if (n_bytes) {
        *n_bytes = i;
    }
    if (flags) {
        *flags = f;
    }
    return n_chars;
}

static inline double
read_sign(const char** sptr)
{
    double sign = 1.0;
    switch (**sptr) {
    case '-':
        sign = -1.0;
        // fallthru
    case '+':
        ++(*sptr);
        // fallthru
    default:
        return sign;
    }
}

double
serd_strtod(const char* str, char** endptr)
{
    double result = 0.0;

    // Point s at the first non-whitespace character
    const char* s = str;
    while (is_space(*s)) { ++s; }

    // Read leading sign if necessary
    const double sign = read_sign(&s);

    // Parse integer part
    for (; is_digit(*s); ++s) {
        result = (result * 10.0) + (*s - '0');
    }

    // Parse fractional part
    if (*s == '.') {
        double denom = 10.0;
        for (++s; is_digit(*s); ++s) {
            result += (*s - '0') / denom;
            denom *= 10.0;
        }
    }

    // Parse exponent
    if (*s == 'e' || *s == 'E') {
        ++s;
        double expt = 0.0;
        double expt_sign = read_sign(&s);
        for (; is_digit(*s); ++s) {
            expt = (expt * 10.0) + (*s - '0');
        }
        result *= pow(10, expt * expt_sign);
    }

    if (endptr) {
        *endptr = (char*)s;
    }

    return result * sign;
}

/**
   Base64 decoding table.
   This is indexed by encoded characters and returns the numeric value used
   for decoding, shifted up by 47 to be in the range of printable ASCII.
   A '$' is a placeholder for characters not in the base64 alphabet.
*/
static const char b64_unmap[] =
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$m$$$ncdefghijkl$$$$$$"
"$/0123456789:;<=>?@ABCDEFGH$$$$$$IJKLMNOPQRSTUVWXYZ[\\]^_`ab$$$$"
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$"
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$";

static inline uint8_t
unmap(const uint8_t in)
{
    return (uint8_t)(b64_unmap[in] - 47);
}

/**
   Decode 4 base64 characters to 3 raw bytes.
*/
static inline size_t
decode_chunk(const uint8_t in[4], uint8_t out[3])
{
    out[0] = (uint8_t)(((unmap(in[0]) << 2)) | unmap(in[1]) >> 4);
    out[1] = (uint8_t)(((unmap(in[1]) << 4) & 0xF0) | unmap(in[2]) >> 2);
    out[2] = (uint8_t)(((unmap(in[2]) << 6) & 0xC0) | unmap(in[3]));
    return 1u + (in[2] != '=') + ((in[2] != '=') && (in[3] != '='));
}

void*
serd_base64_decode(const uint8_t* str, size_t len, size_t* size)
{
    void* buf = malloc((len * 3) / 4 + 2);
    *size = 0;
    for (size_t i = 0, j = 0; i < len; j += 3) {
        uint8_t in[] = "====";
        size_t  n_in = 0;
        for (; i < len && n_in < 4; ++n_in) {
            for (; i < len && !is_base64(str[i]); ++i) {}  // Skip junk
            in[n_in] = str[i++];
        }
        if (n_in > 1) {
            *size += decode_chunk(in, (uint8_t*)buf + j);
        }
    }
    return buf;
}

const uint8_t*
serd_uri_to_path(const uint8_t* uri)
{
    const uint8_t* path = uri;
    if (!is_windows_path(uri) && serd_uri_string_has_scheme(uri)) {
        if (strncmp((const char*)uri, "file:", 5)) {
            fprintf(stderr, "Non-file URI `%s'\n", uri);
            return NULL;
        }
        else if (!strncmp((const char*)uri, "file://localhost/", 17)) {
            path = uri + 16;
        }
        else if (!strncmp((const char*)uri, "file://", 7)) {
            path = uri + 7;
        }
        else {
            fprintf(stderr, "Invalid file URI `%s'\n", uri);
            return NULL;
        }
        if (is_windows_path(path + 1)) {
            ++path;  // Special case for terrible Windows file URIs
        }
    }
    return path;
}

uint8_t*
serd_file_uri_parse(const uint8_t* uri, uint8_t** hostname)
{
    const uint8_t* path = uri;
    if (hostname) {
        *hostname = NULL;
    }
    if (!strncmp((const char*)uri, "file://", 7)) {
        const uint8_t* auth = uri + 7;
        if (*auth == '/') {  // No hostname
            path = auth;
        }
        else {  // Has hostname
            if (!(path = (const uint8_t*)strchr((const char*)auth, '/'))) {
                return NULL;
            }
            if (hostname) {
                *hostname = (uint8_t*)calloc((size_t)(path - auth + 1), 1);
                memcpy(*hostname, auth, (size_t)(path - auth));
            }
        }
    }

    if (is_windows_path(path + 1)) {
        ++path;
    }

    SerdChunk chunk = { NULL, 0 };
    for (const uint8_t* s = path; *s; ++s) {
        if (*s == '%') {
            if (*(s + 1) == '%') {
                serd_chunk_sink("%", 1, &chunk);
                ++s;
            }
            else if (is_hexdig(*(s + 1)) && is_hexdig(*(s + 2))) {
                const uint8_t code[3] = { *(s + 1), *(s + 2), 0 };
                const uint8_t c = (uint8_t)strtoul((const char*)code, NULL, 16);
                serd_chunk_sink(&c, 1, &chunk);
                s += 2;
            }
            else {
                s += 2;  // Junk escape, ignore
            }
        }
        else {
            serd_chunk_sink(s, 1, &chunk);
        }
    }
    return serd_chunk_sink_finish(&chunk);
}

bool
serd_uri_string_has_scheme(const uint8_t* utf8)
{
    // RFC3986: scheme ::= ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
    if (!utf8 || !is_alpha(utf8[0])) {
        return false;  // Invalid scheme initial character, URI is relative
    }

    for (uint8_t c; (c = *++utf8) != '\0';) {
        if (!is_uri_scheme_char(c)) {
            return false;
        }
        else if (c == ':') {
            return true;  // End of scheme
        }
    }

    return false;
}

SerdStatus
serd_uri_parse(const uint8_t* utf8, SerdURI* out)
{
    *out = SERD_URI_NULL;

    const uint8_t* ptr = utf8;

    /* See http://tools.ietf.org/html/rfc3986#section-3
       URI = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
    */

    /* S3.1: scheme ::= ALPHA *( ALPHA / DIGIT / "+" / "-" / "." ) */
    if (is_alpha(*ptr)) {
        for (uint8_t c = *++ptr; true; c = *++ptr) {
            switch (c) {
            case '\0': case '/': case '?': case '#':
                ptr = utf8;
                goto path;  // Relative URI (starts with path by definition)
            case ':':
                out->scheme.buf = utf8;
                out->scheme.len = (size_t)((ptr++) - utf8);
                goto maybe_authority;  // URI with scheme
            case '+': case '-': case '.':
                continue;
            default:
                if (is_alpha(c) || is_digit(c)) {
                    continue;
                }
            }
        }
    }

    /* S3.2: The authority component is preceded by a double slash ("//")
       and is terminated by the next slash ("/"), question mark ("?"),
       or number sign ("#") character, or by the end of the URI.
    */
maybe_authority:
    if (*ptr == '/' && *(ptr + 1) == '/') {
        ptr += 2;
        out->authority.buf = ptr;
        for (uint8_t c; (c = *ptr) != '\0'; ++ptr) {
            switch (c) {
            case '/': goto path;
            case '?': goto query;
            case '#': goto fragment;
            default:
                ++out->authority.len;
            }
        }
    }

    /* RFC3986 S3.3: The path is terminated by the first question mark ("?")
       or number sign ("#") character, or by the end of the URI.
    */
path:
    switch (*ptr) {
    case '?':  goto query;
    case '#':  goto fragment;
    case '\0': goto end;
    default:  break;
    }
    out->path.buf = ptr;
    out->path.len = 0;
    for (uint8_t c; (c = *ptr) != '\0'; ++ptr) {
        switch (c) {
        case '?': goto query;
        case '#': goto fragment;
        default:
            ++out->path.len;
        }
    }

    /* RFC3986 S3.4: The query component is indicated by the first question
       mark ("?") character and terminated by a number sign ("#") character
       or by the end of the URI.
    */
query:
    if (*ptr == '?') {
        out->query.buf = ++ptr;
        for (uint8_t c; (c = *ptr) != '\0'; ++ptr) {
            if (c == '#') {
                goto fragment;
            }
            ++out->query.len;
        }
    }

    /* RFC3986 S3.5: A fragment identifier component is indicated by the
       presence of a number sign ("#") character and terminated by the end
       of the URI.
    */
fragment:
    if (*ptr == '#') {
        out->fragment.buf = ptr;
        while (*ptr++ != '\0') {
            ++out->fragment.len;
        }
    }

end:
    return SERD_SUCCESS;
}

/**
   Remove leading dot components from `path`.
   See http://tools.ietf.org/html/rfc3986#section-5.2.3
   @param up Set to the number of up-references (e.g. "../") trimmed
   @return A pointer to the new start of `path`
*/
static const uint8_t*
remove_dot_segments(const uint8_t* path, size_t len, size_t* up)
{
    const uint8_t* begin = path;
    const uint8_t* const end = path + len;

    *up = 0;
    while (begin < end) {
        switch (begin[0]) {
        case '.':
            switch (begin[1]) {
            case '/':
                begin += 2;  // Chop leading "./"
                break;
            case '.':
                switch (begin[2]) {
                case '\0':
                    ++ * up;
                    begin += 2;  // Chop input ".."
                    break;
                case '/':
                    ++ * up;
                    begin += 3;  // Chop leading "../"
                    break;
                default:
                    return begin;
                }
                break;
            case '\0':
                ++begin;  // Chop input "."
                // fallthru
            default:
                return begin;
            }
            break;
        case '/':
            switch (begin[1]) {
            case '.':
                switch (begin[2]) {
                case '/':
                    begin += 2;  // Leading "/./" => "/"
                    break;
                case '.':
                    switch (begin[3]) {
                    case '/':
                        ++ * up;
                        begin += 3;  // Leading "/../" => "/"
                    }
                    break;
                default:
                    return begin;
                }
            }  // else fall through
        default:
            return begin;  // Finished chopping dot components
        }
    }

    return begin;
}

/// Merge `base` and `path` in-place
static void
merge(SerdChunk* base, SerdChunk* path)
{
    size_t         up;
    const uint8_t* begin = remove_dot_segments(path->buf, path->len, &up);
    const uint8_t* end = path->buf + path->len;

    if (base->len) {
        // Find the up'th last slash
        const uint8_t* base_last = (base->buf + base->len - 1);
        ++up;
        do {
            if (*base_last == '/') {
                --up;
            }
        } while (up > 0 && (--base_last > base->buf));

        // Set path prefix
        base->len = (size_t)(base_last - base->buf + 1);
    }

    // Set path suffix
    path->buf = begin;
    path->len = (size_t)(end - begin);
}

/// See http://tools.ietf.org/html/rfc3986#section-5.2.2
void
serd_uri_resolve(const SerdURI* r, const SerdURI* base, SerdURI* t)
{
    if (!base->scheme.len) {
        *t = *r;  // Don't resolve against non-absolute URIs
        return;
    }

    t->path_base.buf = NULL;
    t->path_base.len = 0;
    if (r->scheme.len) {
        *t = *r;
    }
    else {
        if (r->authority.len) {
            t->authority = r->authority;
            t->path = r->path;
            t->query = r->query;
        }
        else {
            t->path = r->path;
            if (!r->path.len) {
                t->path_base = base->path;
                if (r->query.len) {
                    t->query = r->query;
                }
                else {
                    t->query = base->query;
                }
            }
            else {
                if (r->path.buf[0] != '/') {
                    t->path_base = base->path;
                }
                merge(&t->path_base, &t->path);
                t->query = r->query;
            }
            t->authority = base->authority;
        }
        t->scheme = base->scheme;
        t->fragment = r->fragment;
    }
}

/** Write the path of `uri` starting at index `i` */
static size_t
write_path_tail(SerdSink sink, void* stream, const SerdURI* uri, size_t i)
{
    size_t len = 0;
    if (i < uri->path_base.len) {
        len += sink(uri->path_base.buf + i, uri->path_base.len - i, stream);
    }
    if (uri->path.buf) {
        if (i < uri->path_base.len) {
            len += sink(uri->path.buf, uri->path.len, stream);
        }
        else {
            const size_t j = (i - uri->path_base.len);
            len += sink(uri->path.buf + j, uri->path.len - j, stream);
        }
    }
    return len;
}

/** Write the path of `uri` relative to the path of `base`. */
static size_t
write_rel_path(SerdSink       sink,
    void* stream,
    const SerdURI* uri,
    const SerdURI* base)
{
    const size_t path_len = uri_path_len(uri);
    const size_t base_len = uri_path_len(base);
    const size_t min_len = (path_len < base_len) ? path_len : base_len;

    // Find the last separator common to both paths
    size_t last_shared_sep = 0;
    size_t i = 0;
    for (; i < min_len && uri_path_at(uri, i) == uri_path_at(base, i); ++i) {
        if (uri_path_at(uri, i) == '/') {
            last_shared_sep = i;
        }
    }

    if (i == path_len && i == base_len) {  // Paths are identical
        return 0;
    }

    // Find the number of up references ("..") required
    size_t up = 0;
    for (size_t s = last_shared_sep + 1; s < base_len; ++s) {
        if (uri_path_at(base, s) == '/') {
            ++up;
        }
    }

    // Write up references
    size_t len = 0;
    for (size_t u = 0; u < up; ++u) {
        len += sink("../", 3, stream);
    }

    if (last_shared_sep == 0 && up == 0) {
        len += sink("/", 1, stream);
    }

    // Write suffix
    return len += write_path_tail(sink, stream, uri, last_shared_sep + 1);
}

static uint8_t
serd_uri_path_starts_without_slash(const SerdURI* uri)
{
    return ((uri->path_base.len || uri->path.len) &&
        ((!uri->path_base.len || uri->path_base.buf[0] != '/') &&
            (!uri->path.len || uri->path.buf[0] != '/')));
}

/// See http://tools.ietf.org/html/rfc3986#section-5.3
size_t
serd_uri_serialise_relative(const SerdURI* uri,
    const SerdURI* base,
    const SerdURI* root,
    SerdSink       sink,
    void* stream)
{
    size_t     len = 0;
    const bool relative =
        root ? uri_is_under(uri, root) : uri_is_related(uri, base);

    if (relative) {
        len = write_rel_path(sink, stream, uri, base);
    }
    if (!relative || (!len && base->query.buf)) {
        if (uri->scheme.buf) {
            len += sink(uri->scheme.buf, uri->scheme.len, stream);
            len += sink(":", 1, stream);
        }
        if (uri->authority.buf) {
            len += sink("//", 2, stream);
            len += sink(uri->authority.buf, uri->authority.len, stream);
            if (uri->authority.buf[uri->authority.len - 1] != '/' &&
                serd_uri_path_starts_without_slash(uri)) {
                // Special case: ensure path begins with a slash
                // https://tools.ietf.org/html/rfc3986#section-3.2
                len += sink("/", 1, stream);
            }
        }
        len += write_path_tail(sink, stream, uri, 0);
    }
    if (uri->query.buf) {
        len += sink("?", 1, stream);
        len += sink(uri->query.buf, uri->query.len, stream);
    }
    if (uri->fragment.buf) {
        // Note uri->fragment.buf includes the leading `#'
        len += sink(uri->fragment.buf, uri->fragment.len, stream);
    }
    return len;
}

/// See http://tools.ietf.org/html/rfc3986#section-5.3
size_t
serd_uri_serialise(const SerdURI* uri, SerdSink sink, void* stream)
{
    return serd_uri_serialise_relative(uri, NULL, NULL, sink, stream);
}

typedef struct {
    SerdNode graph;
    SerdNode subject;
    SerdNode predicate;
} WriteContext;

static const WriteContext WRITE_CONTEXT_NULL = {
    { 0, 0, 0, 0, SERD_NOTHING },
    { 0, 0, 0, 0, SERD_NOTHING },
    { 0, 0, 0, 0, SERD_NOTHING }
};

typedef enum {
    SEP_NONE,
    SEP_END_S,       ///< End of a subject ('.')
    SEP_END_P,       ///< End of a predicate (';')
    SEP_END_O,       ///< End of an object (',')
    SEP_S_P,         ///< Between a subject and predicate (whitespace)
    SEP_P_O,         ///< Between a predicate and object (whitespace)
    SEP_ANON_BEGIN,  ///< Start of anonymous node ('[')
    SEP_ANON_END,    ///< End of anonymous node (']')
    SEP_LIST_BEGIN,  ///< Start of list ('(')
    SEP_LIST_SEP,    ///< List separator (whitespace)
    SEP_LIST_END,    ///< End of list (')')
    SEP_GRAPH_BEGIN, ///< Start of graph ('{')
    SEP_GRAPH_END,   ///< End of graph ('}')
    SEP_URI_BEGIN,   ///< URI start quote ('<')
    SEP_URI_END      ///< URI end quote ('>')
} Sep;

typedef struct {
    const char* str;               ///< Sep string
    uint8_t     len;               ///< Length of sep string
    uint8_t     space_before;      ///< Newline before sep
    uint8_t     space_after_node;  ///< Newline after sep if after node
    uint8_t     space_after_sep;   ///< Newline after sep if after sep
} SepRule;

static const SepRule rules[] = {
    { NULL,     0, 0, 0, 0 },
    { " .\n\n", 4, 0, 0, 0 },
    { " ;",     2, 0, 1, 1 },
    { " ,",     2, 0, 1, 0 },
    { NULL,     0, 0, 1, 0 },
    { " ",      1, 0, 0, 0 },
    { "[",      1, 0, 1, 1 },
    { "]",      1, 1, 0, 0 },
    { "(",      1, 0, 0, 0 },
    { NULL,     1, 0, 1, 0 },
    { ")",      1, 1, 0, 0 },
    { " {",     2, 0, 1, 1 },
    { " }",     2, 0, 1, 1 },
    { "<",      1, 0, 0, 0 },
    { ">",      1, 0, 0, 0 },
    { "\n",     1, 0, 1, 0 }
};

struct SerdWriterImpl {
    SerdSyntax    syntax;
    SerdStyle     style;
    SerdEnv* env;
    SerdNode      root_node;
    SerdURI       root_uri;
    SerdURI       base_uri;
    SerdStack     anon_stack;
    SerdByteSink  byte_sink;
    SerdErrorSink error_sink;
    void* error_handle;
    WriteContext  context;
    SerdNode      list_subj;
    unsigned      list_depth;
    unsigned      indent;
    uint8_t* bprefix;
    size_t        bprefix_len;
    Sep           last_sep;
    bool          empty;
};

typedef enum {
    WRITE_STRING,
    WRITE_LONG_STRING
} TextContext;

static bool
write_node(SerdWriter* writer,
    const SerdNode* node,
    const SerdNode* datatype,
    const SerdNode* lang,
    Field              field,
    SerdStatementFlags flags);

static bool
supports_abbrev(const SerdWriter* writer)
{
    return writer->syntax == SERD_TURTLE || writer->syntax == SERD_TRIG;
}

static void
w_err(SerdWriter* writer, SerdStatus st, const char* fmt, ...)
{
    /* TODO: This results in errors with no file information, which is not
       helpful when re-serializing a file (particularly for "undefined
       namespace prefix" errors.  The statement sink API needs to be changed to
       add a Cursor parameter so the source can notify the writer of the
       statement origin for better error reporting. */

    va_list args;
    va_start(args, fmt);
    const SerdError e = { st, (const uint8_t*)"", 0, 0, fmt, &args };
    serd_error(writer->error_sink, writer->error_handle, &e);
    va_end(args);
}

static inline WriteContext*
anon_stack_top(SerdWriter* writer)
{
    assert(!serd_stack_is_empty(&writer->anon_stack));
    return (WriteContext*)(writer->anon_stack.buf
        + writer->anon_stack.size - sizeof(WriteContext));
}

static void
copy_node(SerdNode* dst, const SerdNode* src)
{
    if (src) {
        dst->buf = (uint8_t*)realloc((char*)dst->buf, src->n_bytes + 1);
        dst->n_bytes = src->n_bytes;
        dst->n_chars = src->n_chars;
        dst->flags = src->flags;
        dst->type = src->type;
        memcpy((char*)dst->buf, src->buf, src->n_bytes + 1);
    }
    else {
        dst->type = SERD_NOTHING;
    }
}

static inline size_t
sink(const void* buf, size_t len, SerdWriter* writer)
{
    return serd_byte_sink_write(buf, len, &writer->byte_sink);
}

// Write a single character, as an escape for single byte characters
// (Caller prints any single byte characters that don't need escaping)
static size_t
write_character(SerdWriter* writer, const uint8_t* utf8, size_t* size)
{
    char           escape[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    const uint32_t c = parse_utf8_char(utf8, size);
    switch (*size) {
    case 0:
        w_err(writer, SERD_ERR_BAD_ARG, "invalid UTF-8: %X\n", utf8[0]);
        return sink(replacement_char, sizeof(replacement_char), writer);
    case 1:
        snprintf(escape, sizeof(escape), "\\u%04X", utf8[0]);
        return sink(escape, 6, writer);
    default:
        break;
    }

    if (!(writer->style & SERD_STYLE_ASCII)) {
        // Write UTF-8 character directly to UTF-8 output
        return sink(utf8, *size, writer);
    }

    if (c <= 0xFFFF) {
        snprintf(escape, sizeof(escape), "\\u%04X", c);
        return sink(escape, 6, writer);
    }
    else {
        snprintf(escape, sizeof(escape), "\\U%08X", c);
        return sink(escape, 10, writer);
    }
}

static inline bool
uri_must_escape(const uint8_t c)
{
    switch (c) {
    case ' ': case '"': case '<': case '>': case '\\':
    case '^': case '`': case '{': case '|': case '}':
        return true;
    default:
        return !in_range(c, 0x20, 0x7E);
    }
}

static size_t
write_uri(SerdWriter* writer, const uint8_t* utf8, size_t n_bytes)
{
    size_t len = 0;
    for (size_t i = 0; i < n_bytes;) {
        size_t j = i;  // Index of next character that must be escaped
        for (; j < n_bytes; ++j) {
            if (uri_must_escape(utf8[j])) {
                break;
            }
        }

        // Bulk write all characters up to this special one
        len += sink(&utf8[i], j - i, writer);
        if ((i = j) == n_bytes) {
            break;  // Reached end
        }

        // Write UTF-8 character
        size_t size = 0;
        len += write_character(writer, utf8 + i, &size);
        i += size;
        if (size == 0) {
            // Corrupt input, scan to start of next character
            for (++i; i < n_bytes && (utf8[i] & 0x80); ++i) {}
        }
    }
    return len;
}

static bool
lname_must_escape(const uint8_t c)
{
    /* This arbitrary list of characters, most of which have nothing to do with
       Turtle, must be handled as special cases here because the RDF and SPARQL
       WGs are apparently intent on making the once elegant Turtle a baroque
       and inconsistent mess, throwing elegance and extensibility completely
       out the window for no good reason.

       Note '-', '.', and '_' are also in PN_LOCAL_ESC, but are valid unescaped
       in local names, so they are not escaped here. */

    switch (c) {
    case '\'': case '!': case '#': case '$': case '%': case '&':
    case '(': case ')': case '*': case '+': case ',': case '/':
    case ';': case '=': case '?': case '@': case '~':
        return true;
    }
    return false;
}

static size_t
write_lname(SerdWriter* writer, const uint8_t* utf8, size_t n_bytes)
{
    size_t len = 0;
    for (size_t i = 0; i < n_bytes; ++i) {
        size_t j = i;  // Index of next character that must be escaped
        for (; j < n_bytes; ++j) {
            if (lname_must_escape(utf8[j])) {
                break;
            }
        }

        // Bulk write all characters up to this special one
        len += sink(&utf8[i], j - i, writer);
        if ((i = j) == n_bytes) {
            break;  // Reached end
        }

        // Write escape
        len += sink("\\", 1, writer);
        len += sink(&utf8[i], 1, writer);
    }
    return len;
}

static size_t
write_text(SerdWriter* writer, TextContext ctx,
    const uint8_t* utf8, size_t n_bytes)
{
    size_t len = 0;
    for (size_t i = 0; i < n_bytes;) {
        // Fast bulk write for long strings of printable ASCII
        size_t j = i;
        for (; j < n_bytes; ++j) {
            if (utf8[j] == '\\' || utf8[j] == '"'
                || (!in_range(utf8[j], 0x20, 0x7E))) {
                break;
            }
        }

        len += sink(&utf8[i], j - i, writer);
        if ((i = j) == n_bytes) {
            break;  // Reached end
        }

        const uint8_t in = utf8[i++];
        if (ctx == WRITE_LONG_STRING) {
            switch (in) {
            case '\\': len += sink("\\\\", 2, writer); continue;
            case '\b': len += sink("\\b", 2, writer);  continue;
            case '\n': case '\r': case '\t': case '\f':
                len += sink(&in, 1, writer);  // Write character as-is
                continue;
            case '\"':
                if (i == n_bytes) {  // '"' at string end
                    len += sink("\\\"", 2, writer);
                }
                else {
                    len += sink(&in, 1, writer);
                }
                continue;
            default: break;
            }
        }
        else if (ctx == WRITE_STRING) {
            switch (in) {
            case '\\': len += sink("\\\\", 2, writer); continue;
            case '\n': len += sink("\\n", 2, writer);  continue;
            case '\r': len += sink("\\r", 2, writer);  continue;
            case '\t': len += sink("\\t", 2, writer);  continue;
            case '"':  len += sink("\\\"", 2, writer); continue;
            default: break;
            }
            if (writer->syntax == SERD_TURTLE) {
                switch (in) {
                case '\b': len += sink("\\b", 2, writer); continue;
                case '\f': len += sink("\\f", 2, writer); continue;
                }
            }
        }

        // Write UTF-8 character
        size_t size = 0;
        len += write_character(writer, utf8 + i - 1, &size);
        if (size == 0) {
            // Corrupt input, scan to start of next character
            for (; i < n_bytes && (utf8[i] & 0x80); ++i) {}
        }
        else {
            i += size - 1;
        }
    }
    return len;
}

static size_t
uri_sink(const void* buf, size_t len, void* stream)
{
    return write_uri((SerdWriter*)stream, (const uint8_t*)buf, len);
}

static void
write_newline(SerdWriter* writer)
{
    sink("\n", 1, writer);
    for (unsigned i = 0; i < writer->indent; ++i) {
        sink("\t", 1, writer);
    }
}

static bool
write_sep(SerdWriter* writer, const Sep sep)
{
    const SepRule* rule = &rules[sep];
    if (rule->space_before) {
        write_newline(writer);
    }
    if (rule->str) {
        sink(rule->str, rule->len, writer);
    }
    if ((writer->last_sep && rule->space_after_sep) ||
        (!writer->last_sep && rule->space_after_node)) {
        write_newline(writer);
    }
    else if (writer->last_sep && rule->space_after_node) {
        sink(" ", 1, writer);
    }
    writer->last_sep = sep;
    return true;
}

static SerdStatus
reset_context(SerdWriter* writer, bool graph)
{
    if (graph) {
        writer->context.graph.type = SERD_NOTHING;
    }
    writer->context.subject.type = SERD_NOTHING;
    writer->context.predicate.type = SERD_NOTHING;
    writer->empty = false;
    return SERD_SUCCESS;
}

static SerdStatus
free_context(SerdWriter* writer)
{
    serd_node_free(&writer->context.graph);
    serd_node_free(&writer->context.subject);
    serd_node_free(&writer->context.predicate);
    return reset_context(writer, true);
}

static bool
is_inline_start(const SerdWriter* writer, Field field, SerdStatementFlags flags)
{
    return (supports_abbrev(writer) &&
        ((field == FIELD_SUBJECT && (flags & SERD_ANON_S_BEGIN)) ||
            (field == FIELD_OBJECT && (flags & SERD_ANON_O_BEGIN))));
}

static bool
write_literal(SerdWriter* writer,
    const SerdNode* node,
    const SerdNode* datatype,
    const SerdNode* lang,
    SerdStatementFlags flags)
{
    if (supports_abbrev(writer) && datatype && datatype->buf) {
        const char* type_uri = (const char*)datatype->buf;
        if (!strncmp(type_uri, NS_XSD, sizeof(NS_XSD) - 1) && (
            !strcmp(type_uri + sizeof(NS_XSD) - 1, "boolean") ||
            !strcmp(type_uri + sizeof(NS_XSD) - 1, "integer"))) {
            sink(node->buf, node->n_bytes, writer);
            return true;
        }
        else if (!strncmp(type_uri, NS_XSD, sizeof(NS_XSD) - 1) &&
            !strcmp(type_uri + sizeof(NS_XSD) - 1, "decimal") &&
            strchr((const char*)node->buf, '.') &&
            node->buf[node->n_bytes - 1] != '.') {
            /* xsd:decimal literals without trailing digits, e.g. "5.", can
               not be written bare in Turtle.  We could add a 0 which is
               prettier, but changes the text and breaks round tripping.
            */
            sink(node->buf, node->n_bytes, writer);
            return true;
        }
    }

    if (supports_abbrev(writer)
        && (node->flags & (SERD_HAS_NEWLINE | SERD_HAS_QUOTE))) {
        sink("\"\"\"", 3, writer);
        write_text(writer, WRITE_LONG_STRING, node->buf, node->n_bytes);
        sink("\"\"\"", 3, writer);
    }
    else {
        sink("\"", 1, writer);
        write_text(writer, WRITE_STRING, node->buf, node->n_bytes);
        sink("\"", 1, writer);
    }
    if (lang && lang->buf) {
        sink("@", 1, writer);
        sink(lang->buf, lang->n_bytes, writer);
    }
    else if (datatype && datatype->buf) {
        sink("^^", 2, writer);
        return write_node(writer, datatype, NULL, NULL, FIELD_NONE, flags);
    }
    return true;
}

// Return true iff `buf` is a valid prefixed name suffix
static inline bool
is_name(const uint8_t* buf, const size_t len)
{
    // TODO: This is more strict than it should be.
    for (size_t i = 0; i < len; ++i) {
        if (!(is_alpha(buf[i]) || is_digit(buf[i]))) {
            return false;
        }
    }
    return true;
}

static bool
write_uri_node(SerdWriter* const        writer,
    const SerdNode* node,
    const Field              field,
    const SerdStatementFlags flags)
{
    SerdNode  prefix;
    SerdChunk suffix;

    if (is_inline_start(writer, field, flags)) {
        ++writer->indent;
        write_sep(writer, SEP_ANON_BEGIN);
        sink("== ", 3, writer);
    }

    const bool has_scheme = serd_uri_string_has_scheme(node->buf);
    if (field == FIELD_PREDICATE && supports_abbrev(writer)
        && !strcmp((const char*)node->buf, NS_RDF "type")) {
        return sink("a", 1, writer) == 1;
    }
    else if (supports_abbrev(writer)
        && !strcmp((const char*)node->buf, NS_RDF "nil")) {
        return sink("()", 2, writer) == 2;
    }
    else if (has_scheme && (writer->style & SERD_STYLE_CURIED) &&
        serd_env_qualify(writer->env, node, &prefix, &suffix) &&
        is_name(suffix.buf, suffix.len)) {
        write_uri(writer, prefix.buf, prefix.n_bytes);
        sink(":", 1, writer);
        write_uri(writer, suffix.buf, suffix.len);
        return true;
    }

    write_sep(writer, SEP_URI_BEGIN);
    if (writer->style & SERD_STYLE_RESOLVED) {
        SerdURI in_base_uri, uri, abs_uri;
        serd_env_get_base_uri(writer->env, &in_base_uri);
        serd_uri_parse(node->buf, &uri);
        serd_uri_resolve(&uri, &in_base_uri, &abs_uri);
        bool rooted = uri_is_under(&writer->base_uri, &writer->root_uri);
        SerdURI* root = rooted ? &writer->root_uri : &writer->base_uri;
        if (!uri_is_under(&abs_uri, root) ||
            writer->syntax == SERD_NTRIPLES ||
            writer->syntax == SERD_NQUADS) {
            serd_uri_serialise(&abs_uri, uri_sink, writer);
        }
        else {
            serd_uri_serialise_relative(
                &uri, &writer->base_uri, root, uri_sink, writer);
        }
    }
    else {
        write_uri(writer, node->buf, node->n_bytes);
    }
    write_sep(writer, SEP_URI_END);
    if (is_inline_start(writer, field, flags)) {
        sink(" ;", 2, writer);
        write_newline(writer);
    }
    return true;
}

static bool
write_curie(SerdWriter* const        writer,
    const SerdNode* node,
    const Field              field,
    const SerdStatementFlags flags)
{
    SerdChunk  prefix;
    SerdChunk  suffix;
    SerdStatus st;
    switch (writer->syntax) {
    case SERD_NTRIPLES:
    case SERD_NQUADS:
        if ((st = serd_env_expand(writer->env, node, &prefix, &suffix))) {
            w_err(writer, st, "undefined namespace prefix `%s'\n", node->buf);
            return false;
        }
        write_sep(writer, SEP_URI_BEGIN);
        write_uri(writer, prefix.buf, prefix.len);
        write_uri(writer, suffix.buf, suffix.len);
        write_sep(writer, SEP_URI_END);
        break;
    case SERD_TURTLE:
    case SERD_TRIG:
        if (is_inline_start(writer, field, flags)) {
            ++writer->indent;
            write_sep(writer, SEP_ANON_BEGIN);
            sink("== ", 3, writer);
        }
        write_lname(writer, node->buf, node->n_bytes);
        if (is_inline_start(writer, field, flags)) {
            sink(" ;", 2, writer);
            write_newline(writer);
        }
    }
    return true;
}

static bool
write_blank(SerdWriter* const        writer,
    const SerdNode* node,
    const Field              field,
    const SerdStatementFlags flags)
{
    if (supports_abbrev(writer)) {
        if (is_inline_start(writer, field, flags)) {
            ++writer->indent;
            return write_sep(writer, SEP_ANON_BEGIN);
        }
        else if (field == FIELD_SUBJECT && (flags & SERD_LIST_S_BEGIN)) {
            assert(writer->list_depth == 0);
            copy_node(&writer->list_subj, node);
            ++writer->list_depth;
            ++writer->indent;
            return write_sep(writer, SEP_LIST_BEGIN);
        }
        else if (field == FIELD_OBJECT && (flags & SERD_LIST_O_BEGIN)) {
            ++writer->indent;
            ++writer->list_depth;
            return write_sep(writer, SEP_LIST_BEGIN);
        }
        else if ((field == FIELD_SUBJECT && (flags & SERD_EMPTY_S)) ||
            (field == FIELD_OBJECT && (flags & SERD_EMPTY_O))) {
            return sink("[]", 2, writer) == 2;
        }
    }

    sink("_:", 2, writer);
    if (writer->bprefix && !strncmp((const char*)node->buf,
        (const char*)writer->bprefix,
        writer->bprefix_len)) {
        sink(node->buf + writer->bprefix_len,
            node->n_bytes - writer->bprefix_len,
            writer);
    }
    else {
        sink(node->buf, node->n_bytes, writer);
    }

    return true;
}

static bool
write_node(SerdWriter* writer,
    const SerdNode* node,
    const SerdNode* datatype,
    const SerdNode* lang,
    Field              field,
    SerdStatementFlags flags)
{
    bool ret = false;
    switch (node->type) {
    case SERD_LITERAL:
        ret = write_literal(writer, node, datatype, lang, flags);
        break;
    case SERD_URI:
        ret = write_uri_node(writer, node, field, flags);
        break;
    case SERD_CURIE:
        ret = write_curie(writer, node, field, flags);
        break;
    case SERD_BLANK:
        ret = write_blank(writer, node, field, flags);
    default: break;
    }
    writer->last_sep = SEP_NONE;
    return ret;
}

static inline bool
is_resource(const SerdNode* node)
{
    return node->type > SERD_LITERAL;
}

static void
write_pred(SerdWriter* writer, SerdStatementFlags flags, const SerdNode* pred)
{
    write_node(writer, pred, NULL, NULL, FIELD_PREDICATE, flags);
    write_sep(writer, SEP_P_O);
    copy_node(&writer->context.predicate, pred);
}

static bool
write_list_obj(SerdWriter* writer,
    SerdStatementFlags flags,
    const SerdNode* predicate,
    const SerdNode* object,
    const SerdNode* datatype,
    const SerdNode* lang)
{
    if (!strcmp((const char*)object->buf, NS_RDF "nil")) {
        --writer->indent;
        write_sep(writer, SEP_LIST_END);
        return true;
    }
    else if (!strcmp((const char*)predicate->buf, NS_RDF "first")) {
        write_sep(writer, SEP_LIST_SEP);
        write_node(writer, object, datatype, lang, FIELD_OBJECT, flags);
    }
    return false;
}

SerdStatus
serd_writer_write_statement(SerdWriter* writer,
    SerdStatementFlags flags,
    const SerdNode* graph,
    const SerdNode* subject,
    const SerdNode* predicate,
    const SerdNode* object,
    const SerdNode* datatype,
    const SerdNode* lang)
{
    if (!subject || !predicate || !object
        || !subject->buf || !predicate->buf || !object->buf
        || !is_resource(subject) || !is_resource(predicate)) {
        return SERD_ERR_BAD_ARG;
    }

#define TRY(write_result) \
	if (!(write_result)) { \
		return SERD_ERR_UNKNOWN; \
	}

    switch (writer->syntax) {
    case SERD_NTRIPLES:
    case SERD_NQUADS:
        TRY(write_node(writer, subject, NULL, NULL, FIELD_SUBJECT, flags));
        sink(" ", 1, writer);
        TRY(write_node(writer, predicate, NULL, NULL, FIELD_PREDICATE, flags));
        sink(" ", 1, writer);
        TRY(write_node(writer, object, datatype, lang, FIELD_OBJECT, flags));
        if (writer->syntax == SERD_NQUADS && graph) {
            sink(" ", 1, writer);
            TRY(write_node(writer, graph, datatype, lang, FIELD_GRAPH, flags));
        }
        sink(" .\n", 3, writer);
        return SERD_SUCCESS;
    default:
        break;
    }

    if ((graph && !serd_node_equals(graph, &writer->context.graph)) ||
        (!graph && writer->context.graph.type)) {
        writer->indent = 0;
        if (writer->context.subject.type) {
            write_sep(writer, SEP_END_S);
        }
        if (writer->context.graph.type) {
            write_sep(writer, SEP_GRAPH_END);
        }

        reset_context(writer, true);
        if (graph) {
            TRY(write_node(writer, graph, datatype, lang, FIELD_GRAPH, flags));
            ++writer->indent;
            write_sep(writer, SEP_GRAPH_BEGIN);
            copy_node(&writer->context.graph, graph);
        }
    }

    if ((flags & SERD_LIST_CONT)) {
        if (write_list_obj(writer, flags, predicate, object, datatype, lang)) {
            // Reached end of list
            if (--writer->list_depth == 0 && writer->list_subj.type) {
                reset_context(writer, false);
                serd_node_free(&writer->context.subject);
                writer->context.subject = writer->list_subj;
                writer->list_subj = SERD_NODE_NULL;
            }
            return SERD_SUCCESS;
        }
    }
    else if (serd_node_equals(subject, &writer->context.subject)) {
        if (serd_node_equals(predicate, &writer->context.predicate)) {
            // Abbreviate S P
            if (!(flags & SERD_ANON_O_BEGIN)) {
                ++writer->indent;
            }
            write_sep(writer, SEP_END_O);
            write_node(writer, object, datatype, lang, FIELD_OBJECT, flags);
            if (!(flags & SERD_ANON_O_BEGIN)) {
                --writer->indent;
            }
        }
        else {
            // Abbreviate S
            Sep sep = writer->context.predicate.type ? SEP_END_P : SEP_S_P;
            write_sep(writer, sep);
            write_pred(writer, flags, predicate);
            write_node(writer, object, datatype, lang, FIELD_OBJECT, flags);
        }
    }
    else {
        // No abbreviation
        if (writer->context.subject.type) {
            assert(writer->indent > 0);
            --writer->indent;
            if (serd_stack_is_empty(&writer->anon_stack)) {
                write_sep(writer, SEP_END_S);
            }
        }
        else if (!writer->empty) {
            write_sep(writer, SEP_S_P);
        }

        if (!(flags & SERD_ANON_CONT)) {
            write_node(writer, subject, NULL, NULL, FIELD_SUBJECT, flags);
            ++writer->indent;
            write_sep(writer, SEP_S_P);
        }
        else {
            ++writer->indent;
        }

        reset_context(writer, false);
        copy_node(&writer->context.subject, subject);

        if (!(flags & SERD_LIST_S_BEGIN)) {
            write_pred(writer, flags, predicate);
        }

        write_node(writer, object, datatype, lang, FIELD_OBJECT, flags);
    }

    if (flags & (SERD_ANON_S_BEGIN | SERD_ANON_O_BEGIN)) {
        WriteContext* ctx = (WriteContext*)serd_stack_push(
            &writer->anon_stack, sizeof(WriteContext));
        *ctx = writer->context;
        WriteContext new_context = {
            serd_node_copy(graph), serd_node_copy(subject), SERD_NODE_NULL };
        if ((flags & SERD_ANON_S_BEGIN)) {
            new_context.predicate = serd_node_copy(predicate);
        }
        writer->context = new_context;
    }
    else {
        copy_node(&writer->context.graph, graph);
        copy_node(&writer->context.subject, subject);
        copy_node(&writer->context.predicate, predicate);
    }

    return SERD_SUCCESS;
}

SerdStatus
serd_writer_end_anon(SerdWriter* writer,
    const SerdNode* node)
{
    if (writer->syntax == SERD_NTRIPLES || writer->syntax == SERD_NQUADS) {
        return SERD_SUCCESS;
    }
    if (serd_stack_is_empty(&writer->anon_stack) || writer->indent == 0) {
        w_err(writer, SERD_ERR_UNKNOWN,
            "unexpected end of anonymous node\n");
        return SERD_ERR_UNKNOWN;
    }
    --writer->indent;
    write_sep(writer, SEP_ANON_END);
    free_context(writer);
    writer->context = *anon_stack_top(writer);
    serd_stack_pop(&writer->anon_stack, sizeof(WriteContext));
    const bool is_subject = serd_node_equals(node, &writer->context.subject);
    if (is_subject) {
        copy_node(&writer->context.subject, node);
        writer->context.predicate.type = SERD_NOTHING;
    }
    return SERD_SUCCESS;
}

SerdStatus
serd_writer_finish(SerdWriter* writer)
{
    if (writer->context.subject.type) {
        write_sep(writer, SEP_END_S);
    }
    if (writer->context.graph.type) {
        write_sep(writer, SEP_GRAPH_END);
    }
    serd_byte_sink_flush(&writer->byte_sink);
    writer->indent = 0;
    return free_context(writer);
}

SerdWriter*
serd_writer_new(SerdSyntax     syntax,
    SerdStyle      style,
    SerdEnv* env,
    const SerdURI* base_uri,
    SerdSink       ssink,
    void* stream)
{
    const WriteContext context = WRITE_CONTEXT_NULL;
    SerdWriter* writer = (SerdWriter*)calloc(1, sizeof(SerdWriter));
    writer->syntax = syntax;
    writer->style = style;
    writer->env = env;
    writer->root_node = SERD_NODE_NULL;
    writer->root_uri = SERD_URI_NULL;
    writer->base_uri = base_uri ? *base_uri : SERD_URI_NULL;
    writer->anon_stack = serd_stack_new(4 * sizeof(WriteContext));
    writer->context = context;
    writer->list_subj = SERD_NODE_NULL;
    writer->empty = true;
    writer->byte_sink = serd_byte_sink_new(
        ssink, stream, (style & SERD_STYLE_BULK) ? SERD_PAGE_SIZE : 1);
    return writer;
}

void
serd_writer_set_error_sink(SerdWriter* writer,
    SerdErrorSink error_sink,
    void* error_handle)
{
    writer->error_sink = error_sink;
    writer->error_handle = error_handle;
}

void
serd_writer_chop_blank_prefix(SerdWriter* writer,
    const uint8_t* prefix)
{
    free(writer->bprefix);
    writer->bprefix_len = 0;
    writer->bprefix = NULL;
    if (prefix) {
        writer->bprefix_len = strlen((const char*)prefix);
        writer->bprefix = (uint8_t*)malloc(writer->bprefix_len + 1);
        memcpy(writer->bprefix, prefix, writer->bprefix_len + 1);
    }
}

SerdStatus
serd_writer_set_base_uri(SerdWriter* writer,
    const SerdNode* uri)
{
    if (!serd_env_set_base_uri(writer->env, uri)) {
        serd_env_get_base_uri(writer->env, &writer->base_uri);

        if (writer->syntax == SERD_TURTLE || writer->syntax == SERD_TRIG) {
            if (writer->context.graph.type || writer->context.subject.type) {
                sink(" .\n\n", 4, writer);
                reset_context(writer, true);
            }
            sink("@base <", 7, writer);
            sink(uri->buf, uri->n_bytes, writer);
            sink("> .\n", 4, writer);
        }
        writer->indent = 0;
        return reset_context(writer, true);
    }
    return SERD_ERR_UNKNOWN;
}

SerdStatus
serd_writer_set_root_uri(SerdWriter* writer,
    const SerdNode* uri)
{
    serd_node_free(&writer->root_node);
    if (uri && uri->buf) {
        writer->root_node = serd_node_copy(uri);
        serd_uri_parse(uri->buf, &writer->root_uri);
    }
    else {
        writer->root_node = SERD_NODE_NULL;
        writer->root_uri = SERD_URI_NULL;
    }
    return SERD_SUCCESS;
}

SerdStatus
serd_writer_set_prefix(SerdWriter* writer,
    const SerdNode* name,
    const SerdNode* uri)
{
    if (!serd_env_set_prefix(writer->env, name, uri)) {
        if (writer->syntax == SERD_TURTLE || writer->syntax == SERD_TRIG) {
            if (writer->context.graph.type || writer->context.subject.type) {
                sink(" .\n\n", 4, writer);
                reset_context(writer, true);
            }
            sink("@prefix ", 8, writer);
            sink(name->buf, name->n_bytes, writer);
            sink(": <", 3, writer);
            write_uri(writer, uri->buf, uri->n_bytes);
            sink("> .\n", 4, writer);
        }
        writer->indent = 0;
        return reset_context(writer, true);
    }
    return SERD_ERR_UNKNOWN;
}

void
serd_writer_free(SerdWriter* writer)
{
    serd_writer_finish(writer);
    serd_stack_free(&writer->anon_stack);
    free(writer->bprefix);
    serd_byte_sink_free(&writer->byte_sink);
    serd_node_free(&writer->root_node);
    free(writer);
}

SerdEnv*
serd_writer_get_env(SerdWriter* writer)
{
    return writer->env;
}

size_t
serd_file_sink(const void* buf, size_t len, void* stream)
{
    return fwrite(buf, 1, len, (FILE*)stream);
}

size_t
serd_chunk_sink(const void* buf, size_t len, void* stream)
{
    SerdChunk* chunk = (SerdChunk*)stream;
    chunk->buf = (uint8_t*)realloc((uint8_t*)chunk->buf, chunk->len + len);
    memcpy((uint8_t*)chunk->buf + chunk->len, buf, len);
    chunk->len += len;
    return len;
}

uint8_t*
serd_chunk_sink_finish(SerdChunk* stream)
{
    serd_chunk_sink("", 1, stream);
    return (uint8_t*)stream->buf;
}

int
r_err(SerdReader* reader, SerdStatus st, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    const Cursor* const cur = &reader->source.cur;
    const SerdError e = { st, cur->filename, cur->line, cur->col, fmt, &args };
    serd_error(reader->error_sink, reader->error_handle, &e);
    va_end(args);
    return 0;
}

void
set_blank_id(SerdReader* reader, Ref ref, size_t buf_size)
{
    SerdNode* node = deref(reader, ref);
    const char* prefix = reader->bprefix ? (const char*)reader->bprefix : "";
    node->n_bytes = node->n_chars = (size_t)snprintf(
        (char*)node->buf, buf_size, "%sb%u", prefix, reader->next_id++);
}

size_t
genid_size(SerdReader* reader)
{
    return reader->bprefix_len + 1 + 10 + 1;  // + "b" + UINT32_MAX + \0
}

Ref
blank_id(SerdReader* reader)
{
    Ref ref = push_node_padded(reader, genid_size(reader), SERD_BLANK, "", 0);
    set_blank_id(reader, ref, genid_size(reader));
    return ref;
}

/** fread-like wrapper for getc (which is faster). */
static size_t
serd_file_read_byte(void* buf, size_t size, size_t nmemb, void* stream)
{
    (void)size;
    (void)nmemb;

    const int c = getc((FILE*)stream);
    if (c == EOF) {
        *((uint8_t*)buf) = 0;
        return 0;
    }
    *((uint8_t*)buf) = (uint8_t)c;
    return 1;
}

Ref
push_node_padded(SerdReader* reader, size_t maxlen,
    SerdType type, const char* str, size_t n_bytes)
{
    void* mem = serd_stack_push_aligned(
        &reader->stack, sizeof(SerdNode) + maxlen + 1, sizeof(SerdNode));

    SerdNode* const node = (SerdNode*)mem;
    node->n_bytes = node->n_chars = n_bytes;
    node->flags = 0;
    node->type = type;
    node->buf = NULL;

    uint8_t* buf = (uint8_t*)(node + 1);
    memcpy(buf, str, n_bytes + 1);

#ifdef SERD_STACK_CHECK
    reader->allocs = realloc(
        reader->allocs, sizeof(reader->allocs) * (++reader->n_allocs));
    reader->allocs[reader->n_allocs - 1] = ((uint8_t*)mem - reader->stack.buf);
#endif
    return (Ref)((uint8_t*)node - reader->stack.buf);
}

Ref
push_node(SerdReader* reader, SerdType type, const char* str, size_t n_bytes)
{
    return push_node_padded(reader, n_bytes, type, str, n_bytes);
}

SerdNode*
deref(SerdReader* reader, const Ref ref)
{
    if (ref) {
        SerdNode* node = (SerdNode*)(reader->stack.buf + ref);
        node->buf = (uint8_t*)node + sizeof(SerdNode);
        return node;
    }
    return NULL;
}

Ref
pop_node(SerdReader* reader, Ref ref)
{
    if (ref && ref != reader->rdf_first && ref != reader->rdf_rest
        && ref != reader->rdf_nil) {
#ifdef SERD_STACK_CHECK
        SERD_STACK_ASSERT_TOP(reader, ref);
        --reader->n_allocs;
#endif
        SerdNode* const node = deref(reader, ref);
        uint8_t* const  top = reader->stack.buf + reader->stack.size;
        serd_stack_pop_aligned(&reader->stack, (size_t)(top - (uint8_t*)node));
    }
    return 0;
}

bool
emit_statement(SerdReader* reader, ReadContext ctx, Ref o, Ref d, Ref l)
{
    SerdNode* graph = deref(reader, ctx.graph);
    if (!graph && reader->default_graph.buf) {
        graph = &reader->default_graph;
    }
    bool ret = !reader->statement_sink ||
        !reader->statement_sink(
            reader->handle, *ctx.flags, graph,
            deref(reader, ctx.subject), deref(reader, ctx.predicate),
            deref(reader, o), deref(reader, d), deref(reader, l));
    *ctx.flags &= SERD_ANON_CONT | SERD_LIST_CONT;  // Preserve only cont flags
    return ret;
}

static bool
read_statement(SerdReader* reader)
{
    switch (reader->syntax) {
    default: return read_n3_statement(reader);
    }
}

static bool
read_doc(SerdReader* reader)
{
    return ((reader->syntax == SERD_NQUADS) ? read_nquadsDoc(reader)
        : read_turtleTrigDoc(reader));
}

SerdReader*
serd_reader_new(SerdSyntax        syntax,
    void* handle,
    void              (*free_handle)(void*),
    SerdBaseSink      base_sink,
    SerdPrefixSink    prefix_sink,
    SerdStatementSink statement_sink,
    SerdEndSink       end_sink)
{
    SerdReader* me = (SerdReader*)calloc(1, sizeof(SerdReader));
    me->handle = handle;
    me->free_handle = free_handle;
    me->base_sink = base_sink;
    me->prefix_sink = prefix_sink;
    me->statement_sink = statement_sink;
    me->end_sink = end_sink;
    me->default_graph = SERD_NODE_NULL;
    me->stack = serd_stack_new(SERD_PAGE_SIZE);
    me->syntax = syntax;
    me->next_id = 1;
    me->strict = true;

    me->rdf_first = push_node(me, SERD_URI, NS_RDF "first", 48);
    me->rdf_rest = push_node(me, SERD_URI, NS_RDF "rest", 47);
    me->rdf_nil = push_node(me, SERD_URI, NS_RDF "nil", 46);

    return me;
}

void
serd_reader_set_strict(SerdReader* reader, bool strict)
{
    reader->strict = strict;
}

void
serd_reader_set_error_sink(SerdReader* reader,
    SerdErrorSink error_sink,
    void* error_handle)
{
    reader->error_sink = error_sink;
    reader->error_handle = error_handle;
}

void
serd_reader_free(SerdReader* reader)
{
    pop_node(reader, reader->rdf_nil);
    pop_node(reader, reader->rdf_rest);
    pop_node(reader, reader->rdf_first);
    serd_node_free(&reader->default_graph);

#ifdef SERD_STACK_CHECK
    free(reader->allocs);
#endif
    free(reader->stack.buf);
    free(reader->bprefix);
    if (reader->free_handle) {
        reader->free_handle(reader->handle);
    }
    free(reader);
}

void*
serd_reader_get_handle(const SerdReader* reader)
{
    return reader->handle;
}

void
serd_reader_add_blank_prefix(SerdReader* reader,
    const uint8_t* prefix)
{
    free(reader->bprefix);
    reader->bprefix_len = 0;
    reader->bprefix = NULL;
    if (prefix) {
        reader->bprefix_len = strlen((const char*)prefix);
        reader->bprefix = (uint8_t*)malloc(reader->bprefix_len + 1);
        memcpy(reader->bprefix, prefix, reader->bprefix_len + 1);
    }
}

void
serd_reader_set_default_graph(SerdReader* reader,
    const SerdNode* graph)
{
    serd_node_free(&reader->default_graph);
    reader->default_graph = serd_node_copy(graph);
}

SerdStatus
serd_reader_read_file(SerdReader* reader,
    const uint8_t* uri)
{
    uint8_t* const path = serd_file_uri_parse(uri, NULL);
    if (!path) {
        return SERD_ERR_BAD_ARG;
    }

    FILE* fd = serd_fopen((const char*)path, "rb");
    if (!fd) {
        serd_free(path);
        return SERD_ERR_UNKNOWN;
    }

    SerdStatus ret = serd_reader_read_file_handle(reader, fd, path);
    fclose(fd);
    free(path);
    return ret;
}

static SerdStatus
skip_bom(SerdReader* me)
{
    if (serd_byte_source_peek(&me->source) == 0xEF) {
        serd_byte_source_advance(&me->source);
        if (serd_byte_source_peek(&me->source) != 0xBB ||
            serd_byte_source_advance(&me->source) ||
            serd_byte_source_peek(&me->source) != 0xBF ||
            serd_byte_source_advance(&me->source)) {
            r_err(me, SERD_ERR_BAD_SYNTAX, "corrupt byte order mark\n");
            return SERD_ERR_BAD_SYNTAX;
        }
    }

    return SERD_SUCCESS;
}

SerdStatus
serd_reader_start_stream(SerdReader* reader,
    FILE* file,
    const uint8_t* name,
    bool           bulk)
{
    return serd_reader_start_source_stream(
        reader,
        bulk ? (SerdSource)fread : serd_file_read_byte,
        (SerdStreamErrorFunc)ferror,
        file,
        name,
        bulk ? SERD_PAGE_SIZE : 1);
}

SerdStatus
serd_reader_start_source_stream(SerdReader* reader,
    SerdSource          read_func,
    SerdStreamErrorFunc error_func,
    void* stream,
    const uint8_t* name,
    size_t              page_size)
{
    return serd_byte_source_open_source(
        &reader->source, read_func, error_func, stream, name, page_size);
}

static SerdStatus
serd_reader_prepare(SerdReader* reader)
{
    reader->status = serd_byte_source_prepare(&reader->source);
    if (reader->status == SERD_SUCCESS) {
        reader->status = skip_bom(reader);
    }
    else if (reader->status == SERD_FAILURE) {
        reader->source.eof = true;
    }
    else {
        r_err(reader, reader->status, "read error: %s\n", strerror(errno));
    }
    return reader->status;
}

SerdStatus
serd_reader_read_chunk(SerdReader* reader)
{
    SerdStatus st = SERD_SUCCESS;
    if (!reader->source.prepared) {
        st = serd_reader_prepare(reader);
    }
    else if (reader->source.eof) {
        st = serd_byte_source_advance(&reader->source);
    }

    if (peek_byte(reader) == 0) {
        // Skip leading null byte, for reading from a null-delimited socket
        eat_byte_safe(reader, 0);
    }

    return st ? st : read_statement(reader) ? SERD_SUCCESS : SERD_FAILURE;
}

SerdStatus
serd_reader_end_stream(SerdReader* reader)
{
    return serd_byte_source_close(&reader->source);
}

SerdStatus
serd_reader_read_file_handle(SerdReader* reader,
    FILE* file,
    const uint8_t* name)
{
    return serd_reader_read_source(
        reader, (SerdSource)fread, (SerdStreamErrorFunc)ferror,
        file, name, SERD_PAGE_SIZE);
}

SerdStatus
serd_reader_read_source(SerdReader* reader,
    SerdSource          source,
    SerdStreamErrorFunc error,
    void* stream,
    const uint8_t* name,
    size_t              page_size)
{
    SerdStatus st = serd_reader_start_source_stream(
        reader, source, error, stream, name, page_size);

    if (st || (st = serd_reader_prepare(reader))) {
        serd_reader_end_stream(reader);
        return st;
    }
    else if (!read_doc(reader)) {
        serd_reader_end_stream(reader);
        return SERD_ERR_UNKNOWN;
    }

    return serd_reader_end_stream(reader);
}

SerdStatus
serd_reader_read_string(SerdReader* reader, const uint8_t* utf8)
{
    serd_byte_source_open_string(&reader->source, utf8);

    SerdStatus st = serd_reader_prepare(reader);
    if (!st) {
        st = read_doc(reader) ? SERD_SUCCESS : SERD_ERR_UNKNOWN;
    }

    serd_byte_source_close(&reader->source);

    return st;
}

#ifdef _WIN32
#    ifndef isnan
#        define isnan(x) _isnan(x)
#    endif
#    ifndef isinf
#        define isinf(x) (!_finite(x))
#    endif
#endif

SerdNode
serd_node_from_string(SerdType type, const uint8_t* str)
{
    if (!str) {
        return SERD_NODE_NULL;
    }

    uint32_t     flags = 0;
    size_t       buf_n_bytes = 0;
    const size_t buf_n_chars = serd_strlen(str, &buf_n_bytes, &flags);
    SerdNode ret = { str, buf_n_bytes, buf_n_chars, flags, type };
    return ret;
}

SerdNode
serd_node_from_substring(SerdType type, const uint8_t* str, const size_t len)
{
    if (!str) {
        return SERD_NODE_NULL;
    }

    uint32_t     flags = 0;
    size_t       buf_n_bytes = 0;
    const size_t buf_n_chars = serd_substrlen(str, len, &buf_n_bytes, &flags);
    assert(buf_n_bytes <= len);
    SerdNode ret = { str, buf_n_bytes, buf_n_chars, flags, type };
    return ret;
}

SerdNode
serd_node_copy(const SerdNode* node)
{
    if (!node || !node->buf) {
        return SERD_NODE_NULL;
    }

    SerdNode copy = *node;
    uint8_t* buf = (uint8_t*)malloc(copy.n_bytes + 1);
    memcpy(buf, node->buf, copy.n_bytes + 1);
    copy.buf = buf;
    return copy;
}

bool
serd_node_equals(const SerdNode* a, const SerdNode* b)
{
    return (a == b)
        || (a->type == b->type
            && a->n_bytes == b->n_bytes
            && a->n_chars == b->n_chars
            && ((a->buf == b->buf) || !memcmp((const char*)a->buf,
                (const char*)b->buf,
                a->n_bytes + 1)));
}

static size_t
serd_uri_string_length(const SerdURI* uri)
{
    size_t len = uri->path_base.len;

#define ADD_LEN(field, n_delims) \
	if ((field).len) { len += (field).len + (n_delims); }

    ADD_LEN(uri->path, 1);  // + possible leading `/'
    ADD_LEN(uri->scheme, 1);  // + trailing `:'
    ADD_LEN(uri->authority, 2);  // + leading `//'
    ADD_LEN(uri->query, 1);  // + leading `?'
    ADD_LEN(uri->fragment, 1);  // + leading `#'

    return len + 2;  // + 2 for authority `//'
}

static size_t
string_sink(const void* buf, size_t len, void* stream)
{
    uint8_t** ptr = (uint8_t**)stream;
    memcpy(*ptr, buf, len);
    *ptr += len;
    return len;
}

SerdNode
serd_node_new_uri_from_node(const SerdNode* uri_node,
    const SerdURI* base,
    SerdURI* out)
{
    return (uri_node->type == SERD_URI && uri_node->buf)
        ? serd_node_new_uri_from_string(uri_node->buf, base, out)
        : SERD_NODE_NULL;
}

SerdNode
serd_node_new_uri_from_string(const uint8_t* str,
    const SerdURI* base,
    SerdURI* out)
{
    if (!str || str[0] == '\0') {
        // Empty URI => Base URI, or nothing if no base is given
        return base ? serd_node_new_uri(base, NULL, out) : SERD_NODE_NULL;
    }

    SerdURI uri;
    serd_uri_parse(str, &uri);
    return serd_node_new_uri(&uri, base, out);  // Resolve/Serialise
}

static inline bool
is_uri_path_char(const uint8_t c)
{
    if (is_alpha(c) || is_digit(c)) {
        return true;
    }
    switch (c) {
    case '-': case '.': case '_': case '~':	 // unreserved
    case ':': case '@':	 // pchar
    case '/':  // separator
    // sub-delims
    case '!': case '$': case '&': case '\'': case '(': case ')':
    case '*': case '+': case ',': case ';': case '=':
        return true;
    default:
        return false;
    }
}

SerdNode
serd_node_new_file_uri(const uint8_t* path,
    const uint8_t* hostname,
    SerdURI* out,
    bool           escape)
{
    const size_t path_len = strlen((const char*)path);
    const size_t hostname_len = hostname ? strlen((const char*)hostname) : 0;
    const bool   evil = is_windows_path(path);
    size_t       uri_len = 0;
    uint8_t* uri = NULL;

    if (path[0] == '/' || is_windows_path(path)) {
        uri_len = strlen("file://") + hostname_len + evil;
        uri = (uint8_t*)malloc(uri_len + 1);
        snprintf((char*)uri, uri_len + 1, "file://%s%s",
            hostname ? (const char*)hostname : "",
            evil ? "/" : "");
    }

    SerdChunk chunk = { uri, uri_len };
    for (size_t i = 0; i < path_len; ++i) {
        if (evil && path[i] == '\\') {
            serd_chunk_sink("/", 1, &chunk);
        }
        else if (path[i] == '%') {
            serd_chunk_sink("%%", 2, &chunk);
        }
        else if (!escape || is_uri_path_char(path[i])) {
            serd_chunk_sink(path + i, 1, &chunk);
        }
        else {
            char escape_str[4] = { '%', 0, 0, 0 };
            snprintf(escape_str + 1, sizeof(escape_str) - 1, "%X", path[i]);
            serd_chunk_sink(escape_str, 3, &chunk);
        }
    }
    serd_chunk_sink_finish(&chunk);

    if (out) {
        serd_uri_parse(chunk.buf, out);
    }

    return serd_node_from_substring(SERD_URI, chunk.buf, chunk.len);
}

SerdNode
serd_node_new_uri(const SerdURI* uri, const SerdURI* base, SerdURI* out)
{
    SerdURI abs_uri = *uri;
    if (base) {
        serd_uri_resolve(uri, base, &abs_uri);
    }

    const size_t len = serd_uri_string_length(&abs_uri);
    uint8_t* buf = (uint8_t*)malloc(len + 1);
    SerdNode     node = { buf, 0, 0, 0, SERD_URI };
    uint8_t* ptr = buf;
    const size_t actual_len = serd_uri_serialise(&abs_uri, string_sink, &ptr);

    buf[actual_len] = '\0';
    node.n_bytes = actual_len;
    node.n_chars = serd_strlen(buf, NULL, NULL);

    if (out) {
        serd_uri_parse(buf, out);  // TODO: cleverly avoid double parse
    }

    return node;
}

SerdNode
serd_node_new_relative_uri(const SerdURI* uri,
    const SerdURI* base,
    const SerdURI* root,
    SerdURI* out)
{
    const size_t uri_len = serd_uri_string_length(uri);
    const size_t base_len = serd_uri_string_length(base);
    uint8_t* buf = (uint8_t*)malloc(uri_len + base_len + 1);
    SerdNode     node = { buf, 0, 0, 0, SERD_URI };
    uint8_t* ptr = buf;
    const size_t actual_len = serd_uri_serialise_relative(
        uri, base, root, string_sink, &ptr);

    buf[actual_len] = '\0';
    node.n_bytes = actual_len;
    node.n_chars = serd_strlen(buf, NULL, NULL);

    if (out) {
        serd_uri_parse(buf, out);  // TODO: cleverly avoid double parse
    }

    return node;
}

static inline unsigned
serd_digits(double abs)
{
    const double lg = ceil(log10(floor(abs) + 1.0));
    return lg < 1.0 ? 1U : (unsigned)lg;
}

SerdNode
serd_node_new_decimal(double d, unsigned frac_digits)
{
    if (isnan(d) || isinf(d)) {
        return SERD_NODE_NULL;
    }

    const double   abs_d = fabs(d);
    const unsigned int_digits = serd_digits(abs_d);
    char* buf = (char*)calloc(int_digits + frac_digits + 3, 1);
    SerdNode       node = { (const uint8_t*)buf, 0, 0, 0, SERD_LITERAL };
    const double   int_part = floor(abs_d);

    // Point s to decimal point location
    char* s = buf + int_digits;
    if (d < 0.0) {
        *buf = '-';
        ++s;
    }

    // Write integer part (right to left)
    char* t = s - 1;
    uint64_t dec = (uint64_t)int_part;
    do {
        *t-- = (char)('0' + dec % 10);
    } while ((dec /= 10) > 0);

    *s++ = '.';

    // Write fractional part (right to left)
    double frac_part = fabs(d - int_part);
    if (frac_part < DBL_EPSILON) {
        *s++ = '0';
        node.n_bytes = node.n_chars = (size_t)(s - buf);
    }
    else {
        uint64_t frac = (uint64_t)llround(frac_part * pow(10.0, (int)frac_digits));
        s += frac_digits - 1;
        unsigned i = 0;

        // Skip trailing zeros
        for (; i < frac_digits - 1 && !(frac % 10); ++i, --s, frac /= 10) {}

        node.n_bytes = node.n_chars = (size_t)(s - buf) + 1u;

        // Write digits from last trailing zero to decimal point
        for (; i < frac_digits; ++i) {
            *s-- = (char)('0' + (frac % 10));
            frac /= 10;
        }
    }

    return node;
}

SerdNode
serd_node_new_integer(int64_t i)
{
    int64_t        abs_i = (i < 0) ? -i : i;
    const unsigned digits = serd_digits((double)abs_i);
    char* buf = (char*)calloc(digits + 2, 1);
    SerdNode       node = { (const uint8_t*)buf, 0, 0, 0, SERD_LITERAL };

    // Point s to the end
    char* s = buf + digits - 1;
    if (i < 0) {
        *buf = '-';
        ++s;
    }

    node.n_bytes = node.n_chars = (size_t)(s - buf) + 1u;

    // Write integer part (right to left)
    do {
        *s-- = (char)('0' + (abs_i % 10));
    } while ((abs_i /= 10) > 0);

    return node;
}

/**
   Base64 encoding table.
   @see <a href="http://tools.ietf.org/html/rfc3548#section-3">RFC3986 S3</a>.
*/
static const uint8_t b64_map[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
   Encode 3 raw bytes to 4 base64 characters.
*/
static inline void
encode_chunk(uint8_t out[4], const uint8_t in[3], size_t n_in)
{
    out[0] = b64_map[in[0] >> 2];
    out[1] = b64_map[((in[0] & 0x03) << 4) | ((in[1] & 0xF0) >> 4)];
    out[2] = ((n_in > 1)
        ? (b64_map[((in[1] & 0x0F) << 2) | ((in[2] & 0xC0) >> 6)])
        : (uint8_t)'=');
    out[3] = ((n_in > 2) ? b64_map[in[2] & 0x3F] : (uint8_t)'=');
}

SerdNode
serd_node_new_blob(const void* buf, size_t size, bool wrap_lines)
{
    const size_t len = (size + 2) / 3 * 4 + (wrap_lines * ((size - 1) / 57));
    uint8_t* str = (uint8_t*)calloc(len + 2, 1);
    SerdNode     node = { str, len, len, 0, SERD_LITERAL };
    for (size_t i = 0, j = 0; i < size; i += 3, j += 4) {
        uint8_t in[4] = { 0, 0, 0, 0 };
        size_t  n_in = MIN(3, size - i);
        memcpy(in, (const uint8_t*)buf + i, n_in);

        if (wrap_lines && i > 0 && (i % 57) == 0) {
            str[j++] = '\n';
            node.flags |= SERD_HAS_NEWLINE;
        }

        encode_chunk(str + j, in, n_in);
    }
    return node;
}

void
serd_node_free(SerdNode* node)
{
    if (node && node->buf) {
        free((uint8_t*)node->buf);
        node->buf = NULL;
    }
}

#define TRY_THROW(exp) if (!(exp)) goto except;
#define TRY_RET(exp)   if (!(exp)) return 0;

static inline bool
fancy_syntax(const SerdReader* reader)
{
    return reader->syntax == SERD_TURTLE || reader->syntax == SERD_TRIG;
}

static bool
read_collection(SerdReader* reader, ReadContext ctx, Ref* dest);

static bool
read_predicateObjectList(SerdReader* reader, ReadContext ctx, bool* ate_dot);

static inline uint8_t
read_HEX(SerdReader* reader)
{
    const int c = peek_byte(reader);
    if (is_xdigit(c)) {
        return (uint8_t)eat_byte_safe(reader, c);
    }

    return (uint8_t)r_err(reader, SERD_ERR_BAD_SYNTAX,
        "invalid hexadecimal digit `%c'\n", c);
}

// Read UCHAR escape, initial \ is already eaten by caller
static inline bool
read_UCHAR(SerdReader* reader, Ref dest, uint32_t* char_code)
{
    const int b = peek_byte(reader);
    unsigned  length = 0;
    switch (b) {
    case 'U':
        length = 8;
        break;
    case 'u':
        length = 4;
        break;
    default:
        return false;
    }
    eat_byte_safe(reader, b);

    uint8_t buf[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    for (unsigned i = 0; i < length; ++i) {
        if (!(buf[i] = read_HEX(reader))) {
            return false;
        }
    }

    char* endptr = NULL;
    const uint32_t code = (uint32_t)strtoul((const char*)buf, &endptr, 16);
    assert(endptr == (char*)buf + length);

    unsigned size = 0;
    if (code < 0x00000080) {
        size = 1;
    }
    else if (code < 0x00000800) {
        size = 2;
    }
    else if (code < 0x00010000) {
        size = 3;
    }
    else if (code < 0x00110000) {
        size = 4;
    }
    else {
        r_err(reader, SERD_ERR_BAD_SYNTAX,
            "unicode character 0x%X out of range\n", code);
        push_bytes(reader, dest, replacement_char, 3);
        *char_code = 0xFFFD;
        return true;
    }

    // Build output in buf
    // (Note # of bytes = # of leading 1 bits in first byte)
    uint32_t c = code;
    switch (size) {
    case 4:
        buf[3] = (uint8_t)(0x80u | (c & 0x3Fu));
        c >>= 6;
        c |= (16 << 12);  // set bit 4
        // fallthru
    case 3:
        buf[2] = (uint8_t)(0x80u | (c & 0x3Fu));
        c >>= 6;
        c |= (32 << 6);  // set bit 5
        // fallthru
    case 2:
        buf[1] = (uint8_t)(0x80u | (c & 0x3Fu));
        c >>= 6;
        c |= 0xC0;  // set bits 6 and 7
        // fallthru
    case 1:
        buf[0] = (uint8_t)c;
    }

    push_bytes(reader, dest, buf, size);
    *char_code = code;
    return true;
}

// Read ECHAR escape, initial \ is already eaten by caller
static inline bool
read_ECHAR(SerdReader* reader, Ref dest, SerdNodeFlags* flags)
{
    const int c = peek_byte(reader);
    switch (c) {
    case 't':
        eat_byte_safe(reader, 't');
        push_byte(reader, dest, '\t');
        return true;
    case 'b':
        eat_byte_safe(reader, 'b');
        push_byte(reader, dest, '\b');
        return true;
    case 'n':
        *flags |= SERD_HAS_NEWLINE;
        eat_byte_safe(reader, 'n');
        push_byte(reader, dest, '\n');
        return true;
    case 'r':
        *flags |= SERD_HAS_NEWLINE;
        eat_byte_safe(reader, 'r');
        push_byte(reader, dest, '\r');
        return true;
    case 'f':
        eat_byte_safe(reader, 'f');
        push_byte(reader, dest, '\f');
        return true;
    case '\\': case '"': case '\'':
        push_byte(reader, dest, eat_byte_safe(reader, c));
        return true;
    default:
        return false;
    }
}

static inline SerdStatus
bad_char(SerdReader* reader, const char* fmt, uint8_t c)
{
    // Skip bytes until the next start byte
    for (int b = peek_byte(reader); b != EOF && ((uint8_t)b & 0x80);) {
        eat_byte_safe(reader, b);
        b = peek_byte(reader);
    }

    r_err(reader, SERD_ERR_BAD_SYNTAX, fmt, c);
    return reader->strict ? SERD_ERR_BAD_SYNTAX : SERD_FAILURE;
}

static SerdStatus
read_utf8_bytes(SerdReader* reader, uint8_t bytes[4], uint32_t* size, uint8_t c)
{
    *size = utf8_num_bytes(c);
    if (*size <= 1 || *size > 4) {
        return bad_char(reader, "invalid UTF-8 start 0x%X\n", c);
    }

    bytes[0] = c;
    for (unsigned i = 1; i < *size; ++i) {
        const int b = peek_byte(reader);
        if (b == EOF || ((uint8_t)b & 0x80) == 0) {
            return bad_char(reader, "invalid UTF-8 continuation 0x%X\n",
                (uint8_t)b);
        }

        eat_byte_safe(reader, b);
        bytes[i] = (uint8_t)b;
    }

    return SERD_SUCCESS;
}

static SerdStatus
read_utf8_character(SerdReader* reader, Ref dest, uint8_t c)
{
    uint32_t   size;
    uint8_t    bytes[4];
    SerdStatus st = read_utf8_bytes(reader, bytes, &size, c);
    if (st) {
        push_bytes(reader, dest, replacement_char, 3);
    }
    else {
        push_bytes(reader, dest, bytes, size);
    }
    return st;
}

static SerdStatus
read_utf8_code(SerdReader* reader, Ref dest, uint32_t* code, uint8_t c)
{
    uint32_t   size;
    uint8_t    bytes[4] = { 0, 0, 0, 0 };
    SerdStatus st = read_utf8_bytes(reader, bytes, &size, c);
    if (st) {
        push_bytes(reader, dest, replacement_char, 3);
        return st;
    }

    push_bytes(reader, dest, bytes, size);
    *code = parse_counted_utf8_char(bytes, size);
    return st;
}

// Read one character (possibly multi-byte)
// The first byte, c, has already been eaten by caller
static inline SerdStatus
read_character(SerdReader* reader, Ref dest, SerdNodeFlags* flags, uint8_t c)
{
    if (!(c & 0x80)) {
        switch (c) {
        case 0xA: case 0xD:
            *flags |= SERD_HAS_NEWLINE;
            break;
        case '"': case '\'':
            *flags |= SERD_HAS_QUOTE;
            break;
        }
        push_byte(reader, dest, c);
        return SERD_SUCCESS;
    }
    return read_utf8_character(reader, dest, c);
}

// [10] comment ::= '#' ( [^#xA #xD] )*
static void
read_comment(SerdReader* reader)
{
    eat_byte_safe(reader, '#');
    int c;
    while (((c = peek_byte(reader)) != 0xA) && c != 0xD && c != EOF && c) {
        eat_byte_safe(reader, c);
    }
}

// [24] ws ::= #x9 | #xA | #xD | #x20 | comment
static inline bool
read_ws(SerdReader* reader)
{
    const int c = peek_byte(reader);
    switch (c) {
    case 0x9: case 0xA: case 0xD: case 0x20:
        eat_byte_safe(reader, c);
        return true;
    case '#':
        read_comment(reader);
        return true;
    default:
        return false;
    }
}

static inline bool
read_ws_star(SerdReader* reader)
{
    while (read_ws(reader)) {}
    return true;
}

static inline bool
peek_delim(SerdReader* reader, const char delim)
{
    read_ws_star(reader);
    return peek_byte(reader) == delim;
}

static inline bool
eat_delim(SerdReader* reader, const char delim)
{
    if (peek_delim(reader, delim)) {
        eat_byte_safe(reader, delim);
        return read_ws_star(reader);
    }
    return false;
}

// STRING_LITERAL_LONG_QUOTE and STRING_LITERAL_LONG_SINGLE_QUOTE
// Initial triple quotes are already eaten by caller
static Ref
read_STRING_LITERAL_LONG(SerdReader* reader, SerdNodeFlags* flags, uint8_t q)
{
    Ref        ref = push_node(reader, SERD_LITERAL, "", 0);
    SerdStatus st = SERD_SUCCESS;
    while (!reader->status && !(st && reader->strict)) {
        const int c = peek_byte(reader);
        if (c == '\\') {
            eat_byte_safe(reader, c);
            uint32_t code;
            if (!read_ECHAR(reader, ref, flags) &&
                !read_UCHAR(reader, ref, &code)) {
                r_err(reader, SERD_ERR_BAD_SYNTAX,
                    "invalid escape `\\%c'\n", peek_byte(reader));
                return pop_node(reader, ref);
            }
        }
        else if (c == q) {
            eat_byte_safe(reader, q);
            const int q2 = eat_byte_safe(reader, peek_byte(reader));
            const int q3 = peek_byte(reader);
            if (q2 == q && q3 == q) {  // End of string
                eat_byte_safe(reader, q3);
                break;
            }
            *flags |= SERD_HAS_QUOTE;
            push_byte(reader, ref, c);
            read_character(reader, ref, flags, (uint8_t)q2);
        }
        else if (c == EOF) {
            r_err(reader, SERD_ERR_BAD_SYNTAX, "end of file in long string\n");
            return pop_node(reader, ref);
        }
        else {
            st = read_character(
                reader, ref, flags, (uint8_t)eat_byte_safe(reader, c));
        }
    }
    return ref;
}

// STRING_LITERAL_QUOTE and STRING_LITERAL_SINGLE_QUOTE
// Initial quote is already eaten by caller
static Ref
read_STRING_LITERAL(SerdReader* reader, SerdNodeFlags* flags, uint8_t q)
{
    Ref        ref = push_node(reader, SERD_LITERAL, "", 0);
    SerdStatus st = SERD_SUCCESS;
    while (!reader->status && !(st && reader->strict)) {
        const int c = peek_byte(reader);
        uint32_t  code = 0;
        switch (c) {
        case EOF:
            r_err(reader, SERD_ERR_BAD_SYNTAX, "end of file in short string\n");
            return pop_node(reader, ref);
        case '\n': case '\r':
            r_err(reader, SERD_ERR_BAD_SYNTAX, "line end in short string\n");
            return pop_node(reader, ref);
        case '\\':
            eat_byte_safe(reader, c);
            if (!read_ECHAR(reader, ref, flags) &&
                !read_UCHAR(reader, ref, &code)) {
                r_err(reader, SERD_ERR_BAD_SYNTAX,
                    "invalid escape `\\%c'\n", peek_byte(reader));
                return pop_node(reader, ref);
            }
            break;
        default:
            if (c == q) {
                eat_byte_check(reader, q);
                return ref;
            }
            else {
                st = read_character(
                    reader, ref, flags, (uint8_t)eat_byte_safe(reader, c));
            }
        }
    }
    eat_byte_check(reader, q);
    return ref;
}

static Ref
read_String(SerdReader* reader, SerdNodeFlags* flags)
{
    const int q1 = peek_byte(reader);
    eat_byte_safe(reader, q1);

    const int q2 = peek_byte(reader);
    if (q2 == EOF) {
        return r_err(reader, SERD_ERR_BAD_SYNTAX, "unexpected end of file\n");
    }
    else if (q2 != q1) {  // Short string (not triple quoted)
        return read_STRING_LITERAL(reader, flags, (uint8_t)q1);
    }

    eat_byte_safe(reader, q2);
    const int q3 = peek_byte(reader);
    if (q3 == EOF) {
        return r_err(reader, SERD_ERR_BAD_SYNTAX, "unexpected end of file\n");
    }
    else if (q3 != q1) {  // Empty short string ("" or '')
        return push_node(reader, SERD_LITERAL, "", 0);
    }

    if (!fancy_syntax(reader)) {
        return r_err(reader, SERD_ERR_BAD_SYNTAX,
            "syntax does not support long literals\n");
    }

    eat_byte_safe(reader, q3);
    return read_STRING_LITERAL_LONG(reader, flags, (uint8_t)q1);
}

static inline bool
is_PN_CHARS_BASE(const uint32_t c)
{
    return ((c >= 0x00C0 && c <= 0x00D6) || (c >= 0x00D8 && c <= 0x00F6) ||
        (c >= 0x00F8 && c <= 0x02FF) || (c >= 0x0370 && c <= 0x037D) ||
        (c >= 0x037F && c <= 0x1FFF) || (c >= 0x200C && c <= 0x200D) ||
        (c >= 0x2070 && c <= 0x218F) || (c >= 0x2C00 && c <= 0x2FEF) ||
        (c >= 0x3001 && c <= 0xD7FF) || (c >= 0xF900 && c <= 0xFDCF) ||
        (c >= 0xFDF0 && c <= 0xFFFD) || (c >= 0x10000 && c <= 0xEFFFF));
}

static SerdStatus
read_PN_CHARS_BASE(SerdReader* reader, Ref dest)
{
    uint32_t   code;
    const int  c = peek_byte(reader);
    SerdStatus st = SERD_SUCCESS;
    if (is_alpha(c)) {
        push_byte(reader, dest, eat_byte_safe(reader, c));
    }
    else if (c == EOF || !(c & 0x80)) {
        return SERD_FAILURE;
    }
    else if ((st = read_utf8_code(reader, dest, &code,
        (uint8_t)eat_byte_safe(reader, c)))) {
        return st;
    }
    else if (!is_PN_CHARS_BASE(code)) {
        r_err(reader, SERD_ERR_BAD_SYNTAX,
            "invalid character U+%04X in name\n", code);
        if (reader->strict) {
            return SERD_ERR_BAD_SYNTAX;
        }
    }
    return st;
}

static inline bool
is_PN_CHARS(const uint32_t c)
{
    return (is_PN_CHARS_BASE(c) || c == 0xB7 ||
        (c >= 0x0300 && c <= 0x036F) || (c >= 0x203F && c <= 0x2040));
}

static SerdStatus
read_PN_CHARS(SerdReader* reader, Ref dest)
{
    uint32_t   code;
    const int  c = peek_byte(reader);
    SerdStatus st = SERD_SUCCESS;
    if (is_alpha(c) || is_digit(c) || c == '_' || c == '-') {
        push_byte(reader, dest, eat_byte_safe(reader, c));
    }
    else if (c == EOF || !(c & 0x80)) {
        return SERD_FAILURE;
    }
    else if ((st = read_utf8_code(reader, dest, &code,
        (uint8_t)eat_byte_safe(reader, c)))) {
        return st;
    }
    else if (!is_PN_CHARS(code)) {
        r_err(reader, (st = SERD_ERR_BAD_SYNTAX),
            "invalid character U+%04X in name\n", code);
    }
    return st;
}

static bool
read_PERCENT(SerdReader* reader, Ref dest)
{
    push_byte(reader, dest, eat_byte_safe(reader, '%'));
    const uint8_t h1 = read_HEX(reader);
    const uint8_t h2 = read_HEX(reader);
    if (h1 && h2) {
        push_byte(reader, dest, h1);
        push_byte(reader, dest, h2);
        return true;
    }
    return false;
}

static SerdStatus
read_PLX(SerdReader* reader, Ref dest)
{
    int c = peek_byte(reader);
    switch (c) {
    case '%':
        if (!read_PERCENT(reader, dest)) {
            return SERD_ERR_BAD_SYNTAX;
        }
        return SERD_SUCCESS;
    case '\\':
        eat_byte_safe(reader, c);
        if (is_alpha(c = peek_byte(reader))) {
            // Escapes like \u \n etc. are not supported
            return SERD_ERR_BAD_SYNTAX;
        }
        // Allow escaping of pretty much any other character
        push_byte(reader, dest, eat_byte_safe(reader, c));
        return SERD_SUCCESS;
    default:
        return SERD_FAILURE;
    }
}

static SerdStatus
read_PN_LOCAL(SerdReader* reader, Ref dest, bool* ate_dot)
{
    int        c = peek_byte(reader);
    SerdStatus st = SERD_SUCCESS;
    bool       trailing_unescaped_dot = false;
    switch (c) {
    case '0': case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9': case ':': case '_':
        push_byte(reader, dest, eat_byte_safe(reader, c));
        break;
    default:
        if ((st = read_PLX(reader, dest)) > SERD_FAILURE) {
            return st;
        }
        else if (st != SERD_SUCCESS && read_PN_CHARS_BASE(reader, dest)) {
            return SERD_FAILURE;
        }
    }

    while ((c = peek_byte(reader))) {  // Middle: (PN_CHARS | '.' | ':')*
        if (c == '.' || c == ':') {
            push_byte(reader, dest, eat_byte_safe(reader, c));
        }
        else if ((st = read_PLX(reader, dest)) > SERD_FAILURE) {
            return st;
        }
        else if (st != SERD_SUCCESS && (st = read_PN_CHARS(reader, dest))) {
            break;
        }
        trailing_unescaped_dot = (c == '.');
    }

    SerdNode* const n = deref(reader, dest);
    if (trailing_unescaped_dot) {
        // Ate trailing dot, pop it from stack/node and inform caller
        --n->n_bytes;
        serd_stack_pop(&reader->stack, 1);
        *ate_dot = true;
    }

    return (st > SERD_FAILURE) ? st : SERD_SUCCESS;
}

// Read the remainder of a PN_PREFIX after some initial characters
static SerdStatus
read_PN_PREFIX_tail(SerdReader* reader, Ref dest)
{
    int c;
    while ((c = peek_byte(reader))) {  // Middle: (PN_CHARS | '.')*
        if (c == '.') {
            push_byte(reader, dest, eat_byte_safe(reader, c));
        }
        else if (read_PN_CHARS(reader, dest)) {
            break;
        }
    }

    const SerdNode* const n = deref(reader, dest);
    if (n->buf[n->n_bytes - 1] == '.' && read_PN_CHARS(reader, dest)) {
        r_err(reader, SERD_ERR_BAD_SYNTAX, "prefix ends with `.'\n");
        return SERD_ERR_BAD_SYNTAX;
    }

    return SERD_SUCCESS;
}

static SerdStatus
read_PN_PREFIX(SerdReader* reader, Ref dest)
{
    if (!read_PN_CHARS_BASE(reader, dest)) {
        return read_PN_PREFIX_tail(reader, dest);
    }
    return SERD_FAILURE;
}

static Ref
read_LANGTAG(SerdReader* reader)
{
    int c = peek_byte(reader);
    if (!is_alpha(c)) {
        return r_err(reader, SERD_ERR_BAD_SYNTAX, "unexpected `%c'\n", c);
    }

    Ref ref = push_node(reader, SERD_LITERAL, "", 0);
    push_byte(reader, ref, eat_byte_safe(reader, c));
    while ((c = peek_byte(reader)) && is_alpha(c)) {
        push_byte(reader, ref, eat_byte_safe(reader, c));
    }
    while (peek_byte(reader) == '-') {
        push_byte(reader, ref, eat_byte_safe(reader, '-'));
        while ((c = peek_byte(reader)) && (is_alpha(c) || is_digit(c))) {
            push_byte(reader, ref, eat_byte_safe(reader, c));
        }
    }
    return ref;
}

static bool
read_IRIREF_scheme(SerdReader* reader, Ref dest)
{
    int c = peek_byte(reader);
    if (!isalpha(c)) {
        return r_err(reader, SERD_ERR_BAD_SYNTAX,
            "bad IRI scheme start `%c'\n", c);
    }

    while ((c = peek_byte(reader)) != EOF) {
        if (c == '>') {
            return r_err(reader, SERD_ERR_BAD_SYNTAX, "missing IRI scheme\n");
        }
        else if (!is_uri_scheme_char(c)) {
            return r_err(reader, SERD_ERR_BAD_SYNTAX,
                "bad IRI scheme char `%X'\n", c);
        }

        push_byte(reader, dest, eat_byte_safe(reader, c));
        if (c == ':') {
            return true;  // End of scheme
        }
    }

    return r_err(reader, SERD_ERR_BAD_SYNTAX, "unexpected end of file\n");
}

static Ref
read_IRIREF(SerdReader* reader)
{
    TRY_RET(eat_byte_check(reader, '<'));
    Ref        ref = push_node(reader, SERD_URI, "", 0);
    SerdStatus st = SERD_SUCCESS;
    if (!fancy_syntax(reader) && !read_IRIREF_scheme(reader, ref)) {
        return pop_node(reader, ref);
    }

    uint32_t code = 0;
    while (!reader->status && !(st && reader->strict)) {
        const int c = eat_byte_safe(reader, peek_byte(reader));
        switch (c) {
        case '"': case '<': case '^': case '`': case '{': case '|': case '}':
            r_err(reader, SERD_ERR_BAD_SYNTAX,
                "invalid IRI character `%c'\n", c);
            return pop_node(reader, ref);
        case '>':
            return ref;
        case '\\':
            if (!read_UCHAR(reader, ref, &code)) {
                r_err(reader, SERD_ERR_BAD_SYNTAX, "invalid IRI escape\n");
                return pop_node(reader, ref);
            }
            switch (code) {
            case 0: case ' ': case '<': case '>':
                r_err(reader, SERD_ERR_BAD_SYNTAX,
                    "invalid escaped IRI character %X %c\n", code, code);
                return pop_node(reader, ref);
            }
            break;
        default:
            if (c <= 0x20) {
                if (isprint(c)) {
                    r_err(reader, SERD_ERR_BAD_SYNTAX,
                        "invalid IRI character `%c' (escape %%%02X)\n",
                        c, (unsigned)c);
                }
                else {
                    r_err(reader, SERD_ERR_BAD_SYNTAX,
                        "invalid IRI character (escape %%%02X)\n",
                        (unsigned)c);
                }
                if (reader->strict) {
                    return pop_node(reader, ref);
                }
                reader->status = SERD_FAILURE;
                push_byte(reader, ref, c);
            }
            else if (!(c & 0x80)) {
                push_byte(reader, ref, c);
            }
            else if ((st = read_utf8_character(reader, ref, (uint8_t)c))) {
                if (reader->strict) {
                    reader->status = SERD_FAILURE;
                    return pop_node(reader, ref);
                }
            }
        }
    }
    return pop_node(reader, ref);
}

static bool
read_PrefixedName(SerdReader* reader, Ref dest, bool read_prefix, bool* ate_dot)
{
    if (read_prefix && read_PN_PREFIX(reader, dest) > SERD_FAILURE) {
        return false;
    }
    else if (peek_byte(reader) != ':') {
        return false;
    }

    push_byte(reader, dest, eat_byte_safe(reader, ':'));
    return read_PN_LOCAL(reader, dest, ate_dot) <= SERD_FAILURE;
}

static bool
read_0_9(SerdReader* reader, Ref str, bool at_least_one)
{
    unsigned count = 0;
    for (int c; is_digit((c = peek_byte(reader))); ++count) {
        push_byte(reader, str, eat_byte_safe(reader, c));
    }
    if (at_least_one && count == 0) {
        r_err(reader, SERD_ERR_BAD_SYNTAX, "expected digit\n");
    }
    return count;
}

static bool
read_number(SerdReader* reader, Ref* dest, Ref* datatype, bool* ate_dot)
{
#define XSD_DECIMAL NS_XSD "decimal"
#define XSD_DOUBLE  NS_XSD "double"
#define XSD_INTEGER NS_XSD "integer"

    Ref  ref = push_node(reader, SERD_LITERAL, "", 0);
    int  c = peek_byte(reader);
    bool has_decimal = false;
    if (c == '-' || c == '+') {
        push_byte(reader, ref, eat_byte_safe(reader, c));
    }
    if ((c = peek_byte(reader)) == '.') {
        has_decimal = true;
        // decimal case 2 (e.g. '.0' or `-.0' or `+.0')
        push_byte(reader, ref, eat_byte_safe(reader, c));
        TRY_THROW(read_0_9(reader, ref, true));
    }
    else {
        // all other cases ::= ( '-' | '+' ) [0-9]+ ( . )? ( [0-9]+ )? ...
        TRY_THROW(is_digit(c));
        read_0_9(reader, ref, true);
        if ((c = peek_byte(reader)) == '.') {
            has_decimal = true;

            // Annoyingly, dot can be end of statement, so tentatively eat
            eat_byte_safe(reader, c);
            c = peek_byte(reader);
            if (!is_digit(c) && c != 'e' && c != 'E') {
                *dest = ref;
                *ate_dot = true;  // Force caller to deal with stupid grammar
                return true;  // Next byte is not a number character, done
            }

            push_byte(reader, ref, '.');
            read_0_9(reader, ref, false);
        }
    }
    c = peek_byte(reader);
    if (c == 'e' || c == 'E') {
        // double
        push_byte(reader, ref, eat_byte_safe(reader, c));
        switch ((c = peek_byte(reader))) {
        case '+': case '-':
            push_byte(reader, ref, eat_byte_safe(reader, c));
        default: break;
        }
        TRY_THROW(read_0_9(reader, ref, true));
        *datatype = push_node(reader, SERD_URI,
            XSD_DOUBLE, sizeof(XSD_DOUBLE) - 1);
    }
    else if (has_decimal) {
        *datatype = push_node(reader, SERD_URI,
            XSD_DECIMAL, sizeof(XSD_DECIMAL) - 1);
    }
    else {
        *datatype = push_node(reader, SERD_URI,
            XSD_INTEGER, sizeof(XSD_INTEGER) - 1);
    }
    *dest = ref;
    return true;
except:
    pop_node(reader, *datatype);
    pop_node(reader, ref);
    return r_err(reader, SERD_ERR_BAD_SYNTAX, "bad number syntax\n");
}

static bool
read_iri(SerdReader* reader, Ref* dest, bool* ate_dot)
{
    switch (peek_byte(reader)) {
    case '<':
        *dest = read_IRIREF(reader);
        return true;
    default:
        *dest = push_node(reader, SERD_CURIE, "", 0);
        return read_PrefixedName(reader, *dest, true, ate_dot);
    }
}

static bool
serd_read_literal(SerdReader* reader, Ref* dest,
    Ref* datatype, Ref* lang, SerdNodeFlags* flags, bool* ate_dot)
{
    Ref str = read_String(reader, flags);
    if (!str) {
        return false;
    }

    switch (peek_byte(reader)) {
    case '@':
        eat_byte_safe(reader, '@');
        TRY_THROW(*lang = read_LANGTAG(reader));
        break;
    case '^':
        eat_byte_safe(reader, '^');
        eat_byte_check(reader, '^');
        TRY_THROW(read_iri(reader, datatype, ate_dot));
        break;
    }
    *dest = str;
    return true;
except:
    *datatype = pop_node(reader, *datatype);
    *lang = pop_node(reader, *lang);
    pop_node(reader, str);
    return r_err(reader, SERD_ERR_BAD_SYNTAX, "bad literal syntax\n");
}

static bool
read_verb(SerdReader* reader, Ref* dest)
{
    if (peek_byte(reader) == '<') {
        return (*dest = read_IRIREF(reader));
    }

    /* Either a qname, or "a".  Read the prefix first, and if it is in fact
       "a", produce that instead.
    */
    *dest = push_node(reader, SERD_CURIE, "", 0);
    const SerdStatus st = read_PN_PREFIX(reader, *dest);
    bool             ate_dot = false;
    SerdNode* node = deref(reader, *dest);
    const int        next = peek_byte(reader);
    if (!st && node->n_bytes == 1 && node->buf[0] == 'a' &&
        next != ':' && !is_PN_CHARS_BASE((uint32_t)next)) {
        pop_node(reader, *dest);
        return (*dest = push_node(reader, SERD_URI, NS_RDF "type", 47));
    }
    else if (st > SERD_FAILURE ||
        !read_PrefixedName(reader, *dest, false, &ate_dot) ||
        ate_dot) {
        *dest = pop_node(reader, *dest);
        return r_err(reader, SERD_ERR_BAD_SYNTAX, "bad verb\n");
    }

    return true;
}

static Ref
read_BLANK_NODE_LABEL(SerdReader* reader, bool* ate_dot)
{
    eat_byte_safe(reader, '_');
    eat_byte_check(reader, ':');
    Ref ref = push_node(reader, SERD_BLANK,
        reader->bprefix ? (char*)reader->bprefix : "",
        reader->bprefix_len);

    int c = peek_byte(reader);  // First: (PN_CHARS | '_' | [0-9])
    if (is_digit(c) || c == '_') {
        push_byte(reader, ref, eat_byte_safe(reader, c));
    }
    else if (read_PN_CHARS(reader, ref)) {
        r_err(reader, SERD_ERR_BAD_SYNTAX, "invalid name start character\n");
        return pop_node(reader, ref);
    }

    while ((c = peek_byte(reader))) {  // Middle: (PN_CHARS | '.')*
        if (c == '.') {
            push_byte(reader, ref, eat_byte_safe(reader, c));
        }
        else if (read_PN_CHARS(reader, ref)) {
            break;
        }
    }

    SerdNode* n = deref(reader, ref);
    if (n->buf[n->n_bytes - 1] == '.' && read_PN_CHARS(reader, ref)) {
        // Ate trailing dot, pop it from stack/node and inform caller
        --n->n_bytes;
        serd_stack_pop(&reader->stack, 1);
        *ate_dot = true;
    }

    if (fancy_syntax(reader)) {
        if (is_digit(n->buf[reader->bprefix_len + 1])) {
            if ((n->buf[reader->bprefix_len]) == 'b') {
                ((char*)n->buf)[reader->bprefix_len] = 'B';  // Prevent clash
                reader->seen_genid = true;
            }
            else if (reader->seen_genid &&
                n->buf[reader->bprefix_len] == 'B') {
                r_err(reader, SERD_ERR_ID_CLASH,
                    "found both `b' and `B' blank IDs, prefix required\n");
                return pop_node(reader, ref);
            }
        }
    }
    return ref;
}

static Ref
read_blankName(SerdReader* reader)
{
    eat_byte_safe(reader, '=');
    if (eat_byte_check(reader, '=') != '=') {
        return r_err(reader, SERD_ERR_BAD_SYNTAX, "expected `='\n");
    }

    Ref  subject = 0;
    bool ate_dot = false;
    read_ws_star(reader);
    read_iri(reader, &subject, &ate_dot);
    return subject;
}

static bool
read_anon(SerdReader* reader, ReadContext ctx, bool subject, Ref* dest)
{
    const SerdStatementFlags old_flags = *ctx.flags;
    bool empty;
    eat_byte_safe(reader, '[');
    if ((empty = peek_delim(reader, ']'))) {
        *ctx.flags |= (subject) ? SERD_EMPTY_S : SERD_EMPTY_O;
    }
    else {
        *ctx.flags |= (subject) ? SERD_ANON_S_BEGIN : SERD_ANON_O_BEGIN;
        if (peek_delim(reader, '=')) {
            if (!(*dest = read_blankName(reader)) ||
                !eat_delim(reader, ';')) {
                return false;
            }
        }
    }

    if (!*dest) {
        *dest = blank_id(reader);
    }
    if (ctx.subject) {
        TRY_RET(emit_statement(reader, ctx, *dest, 0, 0));
    }

    ctx.subject = *dest;
    if (!empty) {
        *ctx.flags &= ~(unsigned)SERD_LIST_CONT;
        if (!subject) {
            *ctx.flags |= SERD_ANON_CONT;
        }
        bool ate_dot_in_list = false;
        read_predicateObjectList(reader, ctx, &ate_dot_in_list);
        if (ate_dot_in_list) {
            return r_err(reader, SERD_ERR_BAD_SYNTAX, "`.' inside blank\n");
        }
        read_ws_star(reader);
        if (reader->end_sink) {
            reader->end_sink(reader->handle, deref(reader, *dest));
        }
        *ctx.flags = old_flags;
    }
    return (eat_byte_check(reader, ']') == ']');
}

/* If emit is true: recurses, calling statement_sink for every statement
   encountered, and leaves stack in original calling state (i.e. pops
   everything it pushes). */
static bool
serd_read_object(SerdReader* reader, ReadContext* ctx, bool emit, bool* ate_dot)
{
    static const char* const XSD_BOOLEAN = NS_XSD "boolean";
    static const size_t      XSD_BOOLEAN_LEN = 40;

#ifndef NDEBUG
    const size_t orig_stack_size = reader->stack.size;
#endif

    bool      ret = false;
    bool      simple = (ctx->subject != 0);
    SerdNode* node = NULL;
    Ref       o = 0;
    Ref       datatype = 0;
    Ref       lang = 0;
    uint32_t  flags = 0;
    const int c = peek_byte(reader);
    if (!fancy_syntax(reader)) {
        switch (c) {
        case '"': case ':': case '<': case '_': break;
        default: return r_err(reader, SERD_ERR_BAD_SYNTAX,
            "expected: ':', '<', or '_'\n");
        }
    }
    switch (c) {
    case EOF: case '\0': case ')':
        return r_err(reader, SERD_ERR_BAD_SYNTAX, "expected object\n");
    case '[':
        simple = false;
        TRY_THROW(ret = read_anon(reader, *ctx, false, &o));
        break;
    case '(':
        simple = false;
        TRY_THROW(ret = read_collection(reader, *ctx, &o));
        break;
    case '_':
        TRY_THROW(ret = (o = read_BLANK_NODE_LABEL(reader, ate_dot)));
        break;
    case '<': case ':':
        TRY_THROW(ret = read_iri(reader, &o, ate_dot));
        break;
    case '+': case '-': case '.': case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7': case '8': case '9':
        TRY_THROW(ret = read_number(reader, &o, &datatype, ate_dot));
        break;
    case '\"':
    case '\'':
        TRY_THROW(ret = serd_read_literal(reader, &o, &datatype, &lang, &flags, ate_dot));
        break;
    default:
        /* Either a boolean literal, or a qname.  Read the prefix first, and if
           it is in fact a "true" or "false" literal, produce that instead.
        */
        o = push_node(reader, SERD_CURIE, "", 0);
        while (!read_PN_CHARS_BASE(reader, o)) {}
        node = deref(reader, o);
        if ((node->n_bytes == 4 && !memcmp(node->buf, "true", 4)) ||
            (node->n_bytes == 5 && !memcmp(node->buf, "false", 5))) {
            node->type = SERD_LITERAL;
            datatype = push_node(
                reader, SERD_URI, XSD_BOOLEAN, XSD_BOOLEAN_LEN);
            ret = true;
        }
        else if (read_PN_PREFIX_tail(reader, o) > SERD_FAILURE) {
            ret = false;
        }
        else {
            if (!(ret = read_PrefixedName(reader, o, false, ate_dot))) {
                r_err(reader, SERD_ERR_BAD_SYNTAX, "expected prefixed name\n");
            }
        }
    }

    if (simple && o) {
        deref(reader, o)->flags = flags;
    }

    if (ret && emit && simple) {
        ret = emit_statement(reader, *ctx, o, datatype, lang);
    }
    else if (ret && !emit) {
        ctx->object = o;
        ctx->datatype = datatype;
        ctx->lang = lang;
        return true;
    }

except:
    pop_node(reader, lang);
    pop_node(reader, datatype);
    pop_node(reader, o);
#ifndef NDEBUG
    assert(reader->stack.size == orig_stack_size);
#endif
    return ret;
}

static bool
serd_read_objectList(SerdReader* reader, ReadContext ctx, bool* ate_dot)
{
    TRY_RET(serd_read_object(reader, &ctx, true, ate_dot));
    if (!fancy_syntax(reader) && peek_delim(reader, ',')) {
        return r_err(reader, SERD_ERR_BAD_SYNTAX,
            "syntax does not support abbreviation\n");
    }

    while (!*ate_dot && eat_delim(reader, ',')) {
        TRY_RET(serd_read_object(reader, &ctx, true, ate_dot));
    }
    return true;
}

static bool
read_predicateObjectList(SerdReader* reader, ReadContext ctx, bool* ate_dot)
{
    while (read_verb(reader, &ctx.predicate) &&
        read_ws_star(reader) &&
        serd_read_objectList(reader, ctx, ate_dot)) {
        ctx.predicate = pop_node(reader, ctx.predicate);
        if (*ate_dot) {
            return true;
        }

        bool ate_semi = false;
        int  c;
        do {
            read_ws_star(reader);
            switch (c = peek_byte(reader)) {
            case EOF: case '\0':
                return r_err(reader, SERD_ERR_BAD_SYNTAX,
                    "unexpected end of file\n");
            case '.': case ']': case '}':
                return true;
            case ';':
                eat_byte_safe(reader, c);
                ate_semi = true;
            }
        } while (c == ';');

        if (!ate_semi) {
            return r_err(reader, SERD_ERR_BAD_SYNTAX, "missing ';' or '.'\n");
        }
    }

    return pop_node(reader, ctx.predicate);
}

static bool
end_collection(SerdReader* reader, ReadContext ctx, Ref n1, Ref n2, bool ret)
{
    pop_node(reader, n2);
    pop_node(reader, n1);
    *ctx.flags &= ~(unsigned)SERD_LIST_CONT;
    return ret && (eat_byte_safe(reader, ')') == ')');
}

static bool
read_collection(SerdReader* reader, ReadContext ctx, Ref* dest)
{
    eat_byte_safe(reader, '(');
    bool end = peek_delim(reader, ')');
    *dest = end ? reader->rdf_nil : blank_id(reader);
    if (ctx.subject) {
        // subject predicate _:head
        *ctx.flags |= (end ? 0 : SERD_LIST_O_BEGIN);
        TRY_RET(emit_statement(reader, ctx, *dest, 0, 0));
        *ctx.flags |= SERD_LIST_CONT;
    }
    else {
        *ctx.flags |= (end ? 0 : SERD_LIST_S_BEGIN);
    }

    if (end) {
        return end_collection(reader, ctx, 0, 0, true);
    }

    /* The order of node allocation here is necessarily not in stack order,
       so we create two nodes and recycle them throughout. */
    Ref n1 = push_node_padded(reader, genid_size(reader), SERD_BLANK, "", 0);
    Ref n2 = 0;
    Ref node = n1;
    Ref rest = 0;

    ctx.subject = *dest;
    while (!(end = peek_delim(reader, ')'))) {
        // _:node rdf:first object
        ctx.predicate = reader->rdf_first;
        bool ate_dot = false;
        if (!serd_read_object(reader, &ctx, true, &ate_dot) || ate_dot) {
            return end_collection(reader, ctx, n1, n2, false);
        }

        if (!(end = peek_delim(reader, ')'))) {
            /* Give rest a new ID.  Done as late as possible to ensure it is
               used and > IDs generated by serd_read_object above. */
            if (!rest) {
                rest = n2 = blank_id(reader);  // First pass, push
            }
            else {
                set_blank_id(reader, rest, genid_size(reader));
            }
        }

        // _:node rdf:rest _:rest
        *ctx.flags |= SERD_LIST_CONT;
        ctx.predicate = reader->rdf_rest;
        TRY_RET(emit_statement(reader, ctx,
            (end ? reader->rdf_nil : rest), 0, 0));

        ctx.subject = rest;         // _:node = _:rest
        rest = node;         // _:rest = (old)_:node
        node = ctx.subject;  // invariant
    }

    return end_collection(reader, ctx, n1, n2, true);
}

static Ref
read_subject(SerdReader* reader, ReadContext ctx, Ref* dest, int* s_type)
{
    bool ate_dot = false;
    switch ((*s_type = peek_byte(reader))) {
    case '[':
        read_anon(reader, ctx, true, dest);
        break;
    case '(':
        read_collection(reader, ctx, dest);
        break;
    case '_':
        *dest = read_BLANK_NODE_LABEL(reader, &ate_dot);
        break;
    default:
        TRY_RET(read_iri(reader, dest, &ate_dot));
    }
    return ate_dot ? pop_node(reader, *dest) : *dest;
}

static Ref
read_labelOrSubject(SerdReader* reader)
{
    Ref  subject = 0;
    bool ate_dot = false;
    switch (peek_byte(reader)) {
    case '[':
        eat_byte_safe(reader, '[');
        read_ws_star(reader);
        TRY_RET(eat_byte_check(reader, ']'));
        return blank_id(reader);
    case '_':
        return read_BLANK_NODE_LABEL(reader, &ate_dot);
    default:
        read_iri(reader, &subject, &ate_dot);
    }
    return subject;
}

static bool
read_triples(SerdReader* reader, ReadContext ctx, bool* ate_dot)
{
    bool ret = false;
    if (ctx.subject) {
        read_ws_star(reader);
        switch (peek_byte(reader)) {
        case '.':
            *ate_dot = eat_byte_safe(reader, '.');
            return false;
        case '}':
            return false;
        }
        ret = read_predicateObjectList(reader, ctx, ate_dot);
    }
    ctx.subject = ctx.predicate = 0;
    return ret;
}

static bool
read_base(SerdReader* reader, bool sparql, bool token)
{
    if (token) {
        TRY_RET(eat_string(reader, "base", 4));
    }

    Ref uri;
    read_ws_star(reader);
    TRY_RET(uri = read_IRIREF(reader));
    if (reader->base_sink) {
        reader->base_sink(reader->handle, deref(reader, uri));
    }
    pop_node(reader, uri);

    read_ws_star(reader);
    if (!sparql) {
        return eat_byte_check(reader, '.');
    }
    else if (peek_byte(reader) == '.') {
        return r_err(reader, SERD_ERR_BAD_SYNTAX,
            "full stop after SPARQL BASE\n");
    }
    return true;
}

static bool
read_prefixID(SerdReader* reader, bool sparql, bool token)
{
    if (token) {
        TRY_RET(eat_string(reader, "prefix", 6));
    }

    read_ws_star(reader);
    bool ret = true;
    Ref  name = push_node(reader, SERD_LITERAL, "", 0);
    if (read_PN_PREFIX(reader, name) > SERD_FAILURE) {
        return pop_node(reader, name);
    }

    if (eat_byte_check(reader, ':') != ':') {
        return pop_node(reader, name);
    }

    read_ws_star(reader);
    const Ref uri = read_IRIREF(reader);
    if (!uri) {
        pop_node(reader, name);
        return false;
    }

    if (reader->prefix_sink) {
        ret = !reader->prefix_sink(reader->handle,
            deref(reader, name),
            deref(reader, uri));
    }
    pop_node(reader, uri);
    pop_node(reader, name);
    if (!sparql) {
        read_ws_star(reader);
        return eat_byte_check(reader, '.');
    }
    return ret;
}

static bool
read_directive(SerdReader* reader)
{
    const bool sparql = peek_byte(reader) != '@';
    if (!sparql) {
        eat_byte_safe(reader, '@');
        switch (peek_byte(reader)) {
        case 'B': case 'P':
            return r_err(reader, SERD_ERR_BAD_SYNTAX,
                "uppercase directive\n");
        }
    }

    switch (peek_byte(reader)) {
    case 'B': case 'b': return read_base(reader, sparql, true);
    case 'P': case 'p': return read_prefixID(reader, sparql, true);
    default:
        return r_err(reader, SERD_ERR_BAD_SYNTAX, "invalid directive\n");
    }

    return true;
}

static bool
read_wrappedGraph(SerdReader* reader, ReadContext* ctx)
{
    TRY_RET(eat_byte_check(reader, '{'));
    read_ws_star(reader);
    while (peek_byte(reader) != '}') {
        bool ate_dot = false;
        int  s_type = 0;
        ctx->subject = 0;
        Ref subj = read_subject(reader, *ctx, &ctx->subject, &s_type);
        if (!subj && ctx->subject) {
            return r_err(reader, SERD_ERR_BAD_SYNTAX, "bad subject\n");
        }
        else if (!subj) {
            return false;
        }
        else if (!read_triples(reader, *ctx, &ate_dot) && s_type != '[') {
            return r_err(reader, SERD_ERR_BAD_SYNTAX,
                "missing predicate object list\n");
        }
        pop_node(reader, subj);
        read_ws_star(reader);
        if (peek_byte(reader) == '.') {
            eat_byte_safe(reader, '.');
        }
        read_ws_star(reader);
    }
    return eat_byte_check(reader, '}');
}

static int
tokcmp(SerdReader* reader, Ref ref, const char* tok, size_t n)
{
    SerdNode* node = deref(reader, ref);
    if (!node || node->n_bytes != n) {
        return -1;
    }
    return serd_strncasecmp((const char*)node->buf, tok, n);
}

bool
read_n3_statement(SerdReader* reader)
{
    SerdStatementFlags flags = 0;
    ReadContext        ctx = { 0, 0, 0, 0, 0, 0, &flags };
    Ref                subj = 0;
    bool               ate_dot = false;
    int                s_type = 0;
    bool               ret = true;
    read_ws_star(reader);
    switch (peek_byte(reader)) {
    case EOF: case '\0':
        return reader->status <= SERD_FAILURE;
    case '@':
        if (!fancy_syntax(reader)) {
            return r_err(reader, SERD_ERR_BAD_SYNTAX,
                "syntax does not support directives\n");
        }
        TRY_RET(read_directive(reader));
        read_ws_star(reader);
        break;
    case '{':
        if (reader->syntax == SERD_TRIG) {
            TRY_RET(read_wrappedGraph(reader, &ctx));
            read_ws_star(reader);
        }
        else {
            return r_err(reader, SERD_ERR_BAD_SYNTAX,
                "syntax does not support graphs\n");
        }
        break;
    default:
        subj = read_subject(reader, ctx, &ctx.subject, &s_type);
        if (!tokcmp(reader, ctx.subject, "base", 4)) {
            ret = read_base(reader, true, false);
        }
        else if (!tokcmp(reader, ctx.subject, "prefix", 6)) {
            ret = read_prefixID(reader, true, false);
        }
        else if (!tokcmp(reader, ctx.subject, "graph", 5)) {
            read_ws_star(reader);
            TRY_RET((ctx.graph = read_labelOrSubject(reader)));
            read_ws_star(reader);
            TRY_RET(read_wrappedGraph(reader, &ctx));
            pop_node(reader, ctx.graph);
            ctx.graph = 0;
            read_ws_star(reader);
        }
        else if (read_ws_star(reader) && peek_byte(reader) == '{') {
            if (s_type == '(' || (s_type == '[' && !*ctx.flags)) {
                return r_err(reader, SERD_ERR_BAD_SYNTAX,
                    "invalid graph name\n");
            }
            ctx.graph = subj;
            ctx.subject = subj = 0;
            TRY_RET(read_wrappedGraph(reader, &ctx));
            pop_node(reader, ctx.graph);
            read_ws_star(reader);
        }
        else if (!subj) {
            ret = r_err(reader, SERD_ERR_BAD_SYNTAX, "bad subject\n");
        }
        else if (!read_triples(reader, ctx, &ate_dot)) {
            if (!(ret = (s_type == '[')) && ate_dot) {
                ret = r_err(reader, SERD_ERR_BAD_SYNTAX,
                    "unexpected end of statement\n");
            }
        }
        else if (!ate_dot) {
            read_ws_star(reader);
            ret = (eat_byte_check(reader, '.') == '.');
        }
        pop_node(reader, subj);
        break;
    }
    return ret;
}

static void
skip_until(SerdReader* reader, uint8_t byte)
{
    for (int c = 0; (c = peek_byte(reader)) && c != byte;) {
        eat_byte_safe(reader, c);
    }
}

bool
read_turtleTrigDoc(SerdReader* reader)
{
    while (!reader->source.eof) {
        if (!read_n3_statement(reader)) {
            if (reader->strict) {
                return 0;
            }
            skip_until(reader, '\n');
            reader->status = SERD_SUCCESS;
        }
    }
    return reader->status <= SERD_FAILURE;
}

bool
read_nquadsDoc(SerdReader* reader)
{
    while (!reader->source.eof) {
        SerdStatementFlags flags = 0;
        ReadContext        ctx = { 0, 0, 0, 0, 0, 0, &flags };
        bool               ate_dot = false;
        int                s_type = 0;
        read_ws_star(reader);
        if (peek_byte(reader) == EOF) {
            break;
        }
        else if (peek_byte(reader) == '@') {
            return r_err(reader, SERD_ERR_BAD_SYNTAX,
                "syntax does not support directives\n");
        }

        // subject predicate object
        if (!(ctx.subject = read_subject(reader, ctx, &ctx.subject, &s_type)) ||
            !read_ws_star(reader) ||
            !(ctx.predicate = read_IRIREF(reader)) ||
            !read_ws_star(reader) ||
            !serd_read_object(reader, &ctx, false, &ate_dot)) {
            return false;
        }

        if (!ate_dot) {  // graphLabel?
            TRY_RET(read_ws_star(reader));
            switch (peek_byte(reader)) {
            case '.':
                break;
            case '_':
                ctx.graph = read_BLANK_NODE_LABEL(reader, &ate_dot);
                break;
            default:
                if (!(ctx.graph = read_IRIREF(reader))) {
                    return false;
                }
            }

            // Terminating '.'
            TRY_RET(read_ws_star(reader));
            eat_byte_check(reader, '.');
        }

        TRY_RET(emit_statement(reader, ctx, ctx.object, ctx.datatype, ctx.lang));
        pop_node(reader, ctx.graph);
        pop_node(reader, ctx.lang);
        pop_node(reader, ctx.datatype);
        pop_node(reader, ctx.object);
    }
    return reader->status <= SERD_FAILURE;
}


typedef struct {
    SerdNode name;
    SerdNode uri;
} SerdPrefix;

struct SerdEnvImpl {
    SerdPrefix* prefixes;
    size_t      n_prefixes;
    SerdNode    base_uri_node;
    SerdURI     base_uri;
};

SerdEnv*
serd_env_new(const SerdNode* base_uri)
{
    SerdEnv* env = (SerdEnv*)calloc(1, sizeof(struct SerdEnvImpl));
    if (env && base_uri) {
        serd_env_set_base_uri(env, base_uri);
    }
    return env;
}

void
serd_env_free(SerdEnv* env)
{
    for (size_t i = 0; i < env->n_prefixes; ++i) {
        serd_node_free(&env->prefixes[i].name);
        serd_node_free(&env->prefixes[i].uri);
    }
    free(env->prefixes);
    serd_node_free(&env->base_uri_node);
    free(env);
}

const SerdNode*
serd_env_get_base_uri(const SerdEnv* env,
    SerdURI* out)
{
    if (out) {
        *out = env->base_uri;
    }
    return &env->base_uri_node;
}

SerdStatus
serd_env_set_base_uri(SerdEnv* env,
    const SerdNode* uri)
{
    if (!env || !uri) {
        return SERD_ERR_BAD_ARG;
    }

    // Resolve base URI and create a new node and URI for it
    SerdURI  base_uri;
    SerdNode base_uri_node = serd_node_new_uri_from_node(
        uri, &env->base_uri, &base_uri);

    if (base_uri_node.buf) {
        // Replace the current base URI
        serd_node_free(&env->base_uri_node);
        env->base_uri_node = base_uri_node;
        env->base_uri = base_uri;
        return SERD_SUCCESS;
    }
    return SERD_ERR_BAD_ARG;
}

static inline SerdPrefix*
serd_env_find(const SerdEnv* env,
    const uint8_t* name,
    size_t         name_len)
{
    for (size_t i = 0; i < env->n_prefixes; ++i) {
        const SerdNode* const prefix_name = &env->prefixes[i].name;
        if (prefix_name->n_bytes == name_len) {
            if (!memcmp(prefix_name->buf, name, name_len)) {
                return &env->prefixes[i];
            }
        }
    }
    return NULL;
}

static void
serd_env_add(SerdEnv* env,
    const SerdNode* name,
    const SerdNode* uri)
{
    SerdPrefix* const prefix = serd_env_find(env, name->buf, name->n_bytes);
    if (prefix) {
        SerdNode old_prefix_uri = prefix->uri;
        prefix->uri = serd_node_copy(uri);
        serd_node_free(&old_prefix_uri);
    }
    else {
        env->prefixes = (SerdPrefix*)realloc(
            env->prefixes, (++env->n_prefixes) * sizeof(SerdPrefix));
        env->prefixes[env->n_prefixes - 1].name = serd_node_copy(name);
        env->prefixes[env->n_prefixes - 1].uri = serd_node_copy(uri);
    }
}

SerdStatus
serd_env_set_prefix(SerdEnv* env,
    const SerdNode* name,
    const SerdNode* uri)
{
    if (!name->buf || uri->type != SERD_URI) {
        return SERD_ERR_BAD_ARG;
    }
    else if (serd_uri_string_has_scheme(uri->buf)) {
        // Set prefix to absolute URI
        serd_env_add(env, name, uri);
    }
    else {
        // Resolve relative URI and create a new node and URI for it
        SerdURI  abs_uri;
        SerdNode abs_uri_node = serd_node_new_uri_from_node(
            uri, &env->base_uri, &abs_uri);

        // Set prefix to resolved (absolute) URI
        serd_env_add(env, name, &abs_uri_node);
        serd_node_free(&abs_uri_node);
    }
    return SERD_SUCCESS;
}

SerdStatus
serd_env_set_prefix_from_strings(SerdEnv* env,
    const uint8_t* name,
    const uint8_t* uri)
{
    const SerdNode name_node = serd_node_from_string(SERD_LITERAL, name);
    const SerdNode uri_node = serd_node_from_string(SERD_URI, uri);

    return serd_env_set_prefix(env, &name_node, &uri_node);
}

bool
serd_env_qualify(const SerdEnv* env,
    const SerdNode* uri,
    SerdNode* prefix,
    SerdChunk* suffix)
{
    for (size_t i = 0; i < env->n_prefixes; ++i) {
        const SerdNode* const prefix_uri = &env->prefixes[i].uri;
        if (uri->n_bytes >= prefix_uri->n_bytes) {
            if (!strncmp((const char*)uri->buf,
                (const char*)prefix_uri->buf,
                prefix_uri->n_bytes)) {
                *prefix = env->prefixes[i].name;
                suffix->buf = uri->buf + prefix_uri->n_bytes;
                suffix->len = uri->n_bytes - prefix_uri->n_bytes;
                return true;
            }
        }
    }
    return false;
}

SerdStatus
serd_env_expand(const SerdEnv* env,
    const SerdNode* curie,
    SerdChunk* uri_prefix,
    SerdChunk* uri_suffix)
{
    const uint8_t* const colon = (const uint8_t*)memchr(
        curie->buf, ':', curie->n_bytes + 1);
    if (curie->type != SERD_CURIE || !colon) {
        return SERD_ERR_BAD_ARG;
    }

    const size_t            name_len = (size_t)(colon - curie->buf);
    const SerdPrefix* const prefix = serd_env_find(env, curie->buf, name_len);
    if (prefix) {
        uri_prefix->buf = prefix->uri.buf;
        uri_prefix->len = prefix->uri.n_bytes;
        uri_suffix->buf = colon + 1;
        uri_suffix->len = curie->n_bytes - name_len - 1;
        return SERD_SUCCESS;
    }
    return SERD_ERR_BAD_CURIE;
}

SerdNode
serd_env_expand_node(const SerdEnv* env,
    const SerdNode* node)
{
    switch (node->type) {
    case SERD_CURIE: {
        SerdChunk prefix;
        SerdChunk suffix;
        if (serd_env_expand(env, node, &prefix, &suffix)) {
            return SERD_NODE_NULL;
        }
        const size_t len = prefix.len + suffix.len;
        uint8_t* buf = (uint8_t*)malloc(len + 1);
        SerdNode     ret = { buf, len, 0, 0, SERD_URI };
        snprintf((char*)buf, len + 1, "%s%s", prefix.buf, suffix.buf);
        ret.n_chars = serd_strlen(buf, NULL, NULL);
        return ret;
    }
    case SERD_URI: {
        SerdURI ignored;
        return serd_node_new_uri_from_node(node, &env->base_uri, &ignored);
    }
    default:
        return SERD_NODE_NULL;
    }
}

void
serd_env_foreach(const SerdEnv* env,
    SerdPrefixSink func,
    void* handle)
{
    for (size_t i = 0; i < env->n_prefixes; ++i) {
        func(handle, &env->prefixes[i].name, &env->prefixes[i].uri);
    }
}

SerdStatus
serd_byte_source_page(SerdByteSource* source)
{
    source->read_head = 0;
    const size_t n_read = source->read_func(
        source->file_buf, 1, source->page_size, source->stream);
    if (n_read == 0) {
        source->file_buf[0] = '\0';
        source->eof = true;
        return (source->error_func(source->stream)
            ? SERD_ERR_UNKNOWN : SERD_FAILURE);
    }
    else if (n_read < source->page_size) {
        source->file_buf[n_read] = '\0';
        source->buf_size = n_read;
    }
    return SERD_SUCCESS;
}

SerdStatus
serd_byte_source_open_source(SerdByteSource* source,
    SerdSource          read_func,
    SerdStreamErrorFunc error_func,
    void* stream,
    const uint8_t* name,
    size_t              page_size)
{
    const Cursor cur = { name, 1, 1 };

    memset(source, '\0', sizeof(*source));
    source->stream = stream;
    source->from_stream = true;
    source->page_size = page_size;
    source->buf_size = page_size;
    source->cur = cur;
    source->error_func = error_func;
    source->read_func = read_func;

    if (page_size > 1) {
        source->file_buf = (uint8_t*)serd_bufalloc(page_size);
        source->read_buf = source->file_buf;
        memset(source->file_buf, '\0', page_size);
    }
    else {
        source->read_buf = &source->read_byte;
    }

    return SERD_SUCCESS;
}

SerdStatus
serd_byte_source_prepare(SerdByteSource* source)
{
    source->prepared = true;
    if (source->from_stream) {
        if (source->page_size > 1) {
            return serd_byte_source_page(source);
        }
        else if (source->from_stream) {
            return serd_byte_source_advance(source);
        }
    }
    return SERD_SUCCESS;
}

SerdStatus
serd_byte_source_open_string(SerdByteSource* source, const uint8_t* utf8)
{
    const Cursor cur = { (const uint8_t*)"(string)", 1, 1 };

    memset(source, '\0', sizeof(*source));
    source->cur = cur;
    source->read_buf = utf8;
    return SERD_SUCCESS;
}

SerdStatus
serd_byte_source_close(SerdByteSource* source)
{
    if (source->page_size > 1) {
        free(source->file_buf);
    }
    memset(source, '\0', sizeof(*source));
    return SERD_SUCCESS;
}
