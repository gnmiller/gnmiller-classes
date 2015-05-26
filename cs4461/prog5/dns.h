/* structs for dns header and various other info */
/* uses bit fields to ensure no shenanigans with compilation padding bits etc */
typedef struct _t_dns_header
{
    unsigned short id;          // identification number

    unsigned char rd :1;          // recursion desired
    unsigned char tc :1;          // truncated message
    unsigned char aa :1;          // authoritive answer
    unsigned char opcode :4;          // purpose of message
    unsigned char qr :1;          // query/response flag

    unsigned char rcode :4;          // response code
    unsigned char cd :1;          // checking disabled
    unsigned char ad :1;          // authenticated data
    unsigned char z :1;          // its z! reserved
    unsigned char ra :1;          // recursion available

    unsigned short q_count;          // number of question entries
    unsigned short ans_count;          // number of answer entries
    unsigned short auth_count;          // number of authority entries
    unsigned short add_count;          // number of resource entries
} dns_header_t;

/* map the query into fields */
typedef struct _t_question
{
    unsigned short qtype;
    unsigned short qclass;
} question_t;

/* map the resource structure into fields */
/* force alignment for accuracy with #pragma */
#pragma pack(push, 1)
typedef struct _t_res_data
{
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
} res_data_t;
#pragma pack(pop)

/* used for resource records */
typedef struct _t_res
{
    unsigned char *name;
    res_data_t *resource;
    unsigned char *rdata;
} res_t;

/* used for query */
typedef struct _t_query
{
    unsigned char *name;
    question_t *ques;
} query_t;

/* function prototypes */
void fill_sockaddr( struct sockaddr_in* );
void fill_dnshdr( dns_header_t* );
void set_domain(unsigned char*, unsigned char*);
char *read_name(unsigned char*, unsigned char*, int*);
void parse_ans( char*, int );
void print_ans( int, char* );