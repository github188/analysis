#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


static char *ngx_http_hello_world(ngx_conf_t *cf,
                                  ngx_command_t *cmd,
                                  void *conf);

static ngx_command_t ngx_http_hello_world_commands[] = {
    {
        ngx_string("hello_world"),
        NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,
        ngx_http_hello_world,
        0,
        0,
        NULL
    },
    ngx_null_command,
};

static u_char ngx_hello_world[]
    = "<html><head><title>a</title></head><body>hello world</body></html>";

static ngx_http_module_t ngx_http_hello_world_module_ctx = {
    NULL,
    NULL,

    NULL,
    NULL,

    NULL,
    NULL,

    NULL,
    NULL,
};

ngx_module_t ngx_http_hello_world_module = {
    NGX_MODULE_V1,
    &ngx_http_hello_world_module_ctx,
    ngx_http_hello_world_commands,
    NGX_HTTP_MODULE,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NGX_MODULE_V1_PADDING,
};

static ngx_int_t ngx_http_hello_world_handler(ngx_http_request_t *r)
{
    ngx_buf_t *b = NULL;
    ngx_chain_t out;

    #define TEXT_PLAIN  "text/html"
    r->headers_out.content_type.len = sizeof(TEXT_PLAIN) - 1;
    r->headers_out.content_type.data = (u_char *)TEXT_PLAIN;
    #undef TEXT_PLAIN

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));

    out.buf = b;
    out.next = NULL;

    b->pos = ngx_hello_world;
    b->last = ngx_hello_world + sizeof(ngx_hello_world);
    b->memory = 1;
    b->last_buf = 1;

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = sizeof(ngx_hello_world);

    ngx_http_send_header(r);

    return ngx_http_output_filter(r, &out);
}

static char *ngx_http_hello_world(ngx_conf_t *cf,
                                  ngx_command_t *cmd,
                                  void *conf)
{
    ngx_http_core_loc_conf_t *clcf = NULL;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_hello_world_handler;

    return NGX_CONF_OK;
}
