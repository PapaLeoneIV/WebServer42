#include "../includes/Request.hpp"
#include "../includes/Utils.hpp"
#include "../includes/Logger.hpp"
#include "../includes/ConfigParser.hpp"

/**
 * After we user recv(), we pass the req to consume().
 * It will analyze the req character by character, trying to
 * respect the HTTP protocol standard for req firstline, headers and body.
 * While doing so, it keeps track of the various states of the req and where it
 * did stop parsing. So that, it the req is chunked, or
 * if the Client sended just a portion of the req, and will send the next one later,
 * consume() will be able to resume parsing where it previously stopped.
 */

int Request::consume(const std::string &buffer)
{
    for (size_t i = 0; i < buffer.size(); i++)
    {
        char character = buffer[i];
        switch (this->state)
        {
        case StateMethod:
        {
            if (character != ' ')
            {
                this->raw += character;
                this->content += character;
                continue;
            }

            this->method = this->content;
            this->content.clear();

            if (this->method == this->methods[GET] ||
                this->method == this->methods[POST] ||
                this->method == this->methods[DELETE])
            {
                this->state = StateSpaceAfterMethod;
            }
            else
            {
                this->error = 501; // Not Implemented
                this->state = StateParsingError;
                return 1;
            }
            i--;
            continue;
        }
        case StateSpaceAfterMethod:
        {
            this->raw += character;
            if (character != ' ')
            {
                this->error = 400; // req not valid
                this->state = StateParsingError;
                return 1;
            }
            this->state = StateUrlBegin;
            continue;
        }
        case StateUrlBegin:
        {
            this->raw += character;
            if (character != '/')
            {
                this->error = 400; // req not valid
                this->state = StateParsingError;
                return 1;
            }
            this->content.clear();
            this->content += character;
            this->state = StateUrlString;
            continue;
        }
        case StateUrlString:
        {
            this->raw += character;
            // https://datatracker.ietf.org/doc/html/rfc1738#section-2.1
            if ((character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') || (character >= '0' && character <= '9') || (character == '+') || (character == '.') || (character == '-') || (character == '_') || (character == '!') || (character == '$') || (character == '*') || (character == '\'') || (character == '(') || (character == ')') || (character == '/') || (character == '=') || (character == '&'))
            {
                this->content += character;
                continue;
            }
            if (character == '?')
            {
                this->setUrl(this->content);
                if (this->getUrl().size() > 4 * 1024)
                {                      // url cannot be longer than 4MB
                    this->error = 414; // URI too long
                    this->state = StateParsingError;
                    return 1;
                }
                this->content.clear();
                this->state = StateUrlQuery;
                continue;
            }

            //% https://datatracker.ietf.org/doc/html/rfc3986#section-2.1 HTTP/1.1
            if (character == '%')
            {
                this->state = StateEncodedSep;
                continue;
            }

            if (character == ' ')
            {
                this->state = StateSpaceAfterUrl;
                continue;
            }
            // se arriviamo qui, il carattere non è valido per un URL
            Logger::error("ResourceValidator", "Bad character in URL: '" + std::string(1, character) + "', ASCII: " + wb_itos(static_cast<int>(character)));
            this->error = 400;
            this->state = StateParsingError;
            return 1;
        }
        case StateUrlQuery:
        {
            this->raw += character;
            this->content += character;
            if (character == ' ')
            {
                this->setQueryParam(this->content);
                this->state = StateSpaceAfterUrl;
                continue;
            }
            continue;
        }
        case StateEncodedSep:
        {
            this->raw += character;

            if (this->encoded_counter == 1)
            {
                this->encoded_char += character;

                int hex = wb_stox(this->encoded_char);
                if (hex == 0x0)
                {
                    this->error = 400; // req not valid
                    this->state = StateParsingError;
                    return 1;
                }
                this->encoded_char = static_cast<char>(hex);

                this->content += this->encoded_char;

                this->encoded_counter = 0;
                this->encoded_char.clear();
                this->state = StateUrlString;
                continue;
            }
            this->encoded_char += character;
            this->encoded_counter++;
            continue;
        }

        case StateSpaceAfterUrl:
        {
            this->raw += character;
            if (character != 'H')
            {
                this->error = 400; // req not valid
                this->state = StateParsingError;
                return 1;
            }
            this->setUrl(this->content);
            if (this->getUrl().size() > 4 * 1024)
            {                      // url cannot be longer than 4MB
                this->error = 414; // URI too long
                this->state = StateParsingError;
                return 1;
            }
            this->content.clear();
            this->state = StateVersion_H;
            continue;
        }
        //"HTTP/1.1 only version admitted
        case StateVersion_H:
        {
            this->raw += character;
            if (character != 'T')
            {
                this->error = 505; // HTTP Version Not Supported
                this->state = StateParsingError;
                return 1;
            }
            this->state = StateVersion_HT;
            continue;
        }
        case StateVersion_HT:
        {
            this->raw += character;
            if (character != 'T')
            {
                this->error = 505;
                this->state = StateParsingError;
                return 1;
            }
            this->state = StateVersion_HTT;
            continue;
        }
        case StateVersion_HTT:
        {
            this->raw += character;
            if (character != 'P')
            {
                this->error = 505;
                this->state = StateParsingError;
                return 1;
            }
            this->state = StateVersion_HTTP;
            continue;
        }
        case StateVersion_HTTP:
        {
            this->raw += character;
            if (character != '/')
            {
                this->error = 505;
                this->state = StateParsingError;
                return 1;
            }
            this->state = StateVersion_HTTPSlash;
            continue;
        }
        case StateVersion_HTTPSlash:
        {
            this->raw += character;
            if (character != '1')
            {
                this->error = 505;
                this->state = StateParsingError;
                return 1;
            }
            this->state = StateVersion_HTTPSlashOne;
            continue;
        }
        case StateVersion_HTTPSlashOne:
        {
            this->raw += character;
            if (character != '.')
            {
                this->error = 505;
                this->state = StateParsingError;
                return 1;
            }
            this->state = StateVersion_HTTPSlashOneDot;
            continue;
        }
        case StateVersion_HTTPSlashOneDot:
        {
            this->raw += character;
            if (character != '1')
            {
                this->error = 505;
                this->state = StateParsingError;
                return 1;
            }
            this->state = StateVersion_HTTPSlashOneDotOne;
            continue;
        }
        case StateVersion_HTTPSlashOneDotOne:
        {
            this->raw += character;
            if (character != '\r')
            {
                this->error = 400; // Bad Request
                this->state = StateParsingError;
                return 1;
            }
            this->version = this->content;
            this->content.clear();
            this->state = StateFirstLine_CR;
            continue;
        }
        case StateFirstLine_CR:
        {
            this->raw += character;
            if (character != '\n')
            {
                this->error = 400;
                this->state = StateParsingError;
                return 1;
            }
            this->content.clear();
            this->state = StateHeaderKey;
            continue;
        }
        // HTTP headers are structured such that each header field consists of a case-insensitive field name followed by a colon (:),
        //  optional leading whitespace, the field value, and optional trailing whitespace.They are serialized into a single string where
        //  individual header fields are separated by CRLF (carriage return 1 and line feed, represented by \r\n in many programming languages).
        case StateHeaderKey:
        {
            this->raw += character;
            if (character == ':')
            {
                this->headers_key = this->content;
                this->content.clear();
                this->state = StateHeadersTrailingSpaceStart;
                continue;
            }
            if (character == ' ')
            {
                this->error = 400; // Bad Request
                this->state = StateParsingError;
                return 1;
            }
            if ((character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') || (character >= '0' && character <= '9') || (character == '_') || (character == '-'))
            {
                this->content += character;
                continue;
            }
            std::cout << "this->error: character not allowed" << std::endl;
            this->error = 400; // Bad Request
            this->state = StateParsingError;
            return 1;
        }
        case StateHeadersTrailingSpaceStart:
        {
            this->raw += character;
            if (character == ' ')
            {
                continue;
            }
            this->content += character;
            this->state = StateHeaderValue;
            continue;
        }
        case StateHeaderValue:
        {
            this->raw += character;

            if (character == '\r')
            {
                // Trim trailing whitespace from value
                std::string trimmed_value = trimLeftRight(this->content);
                this->headers[to_lower(this->headers_key)] = to_lower(trimmed_value);
                this->content.clear();
                this->state = StateHeaders_CR;
                continue;
            }

            if (character < 32 || character > 126)
            {
                this->error = 400; // Bad Request
                this->state = StateParsingError;
                return 1;
            }

            this->content += character;
            continue;
        }

        case StateHeadersTrailingSpaceEnd:
        {
            this->raw += character;

            if (character == ' ')
            {
                continue;
            }
            if (character == '\r')
            {
                this->state = StateHeaders_CR;
                continue;
            }
            this->state = StateHeaderValue;
            continue;
        }
        case StateHeaders_CR:
        {
            this->raw += character;
            if (character == '\n')
            {
                this->state = StateHeaders_LF;
                continue;
            }
            this->error = 400; // Bad Request
            this->state = StateParsingError;
            return 1;
        }
        case StateHeaders_LF:
        {
            this->raw += character;
            if (character == '\r')
            {
                this->state = StateHeadersEnd_CR;
                this->raw += character;
                continue;
            }
            this->content += character;
            this->state = StateHeaderKey;
            continue;
        }
        case StateHeadersEnd_CR:
        {
            this->raw += character;
            if (this->method == "POST")
                if (character != '\n')
                {
                    this->error = 400; // Bad Request
                    this->state = StateParsingError;
                    return 1;
                }
            if (!this->headers["content-length"].empty())
            {
                this->has_body = true;
                this->is_chunked = false;
            }
            else if (!this->headers["transfer-encoding"].empty())
            {
                this->has_body = true;
                this->is_chunked = true;
            }
            if (this->has_body && this->method == "POST")
            {
                if (this->is_chunked)
                    this->state = StateBodyChunkedNumber;
                else
                    this->state = StateBodyPlainText;
                this->content.clear();
                continue;
            }
            this->content.clear();
            this->state = StateParsingComplete;
            continue;
        }
        case StateBodyPlainText:
        {
            this->raw += character;
            this->body_counter++;
            this->body += character;
            if (this->body_counter == wb_stoi(this->headers["content-length"]))
            {
                this->state = StateParsingComplete;
                continue;
            }
            if (this->body_counter < wb_stoi(this->headers["content-length"]))
            {
                continue;
            }
            continue;
        }
        case StateBodyChunkedNumber:
        {
            this->raw += character;
            if (character == '\r')
            {
                std::stringstream ss;
                ss << std::hex << this->content;
                ss >> this->number;
                if (!(ss >> this->number))
                {
                    this->error = 400; // conversione in hex fallita
                    this->state = StateParsingError;
                    return 1;
                }
                if (this->number == 0)
                {
                    this->content.clear();
                    this->state = StateChunkedEnd_LF;
                    continue;
                }
                this->content.clear();
                this->state = StateChunkedNumber_LF;
                continue;
            }
            if (!isxdigit(character))
            {
                this->error = 400; // carattere non esadecimale
                this->state = StateParsingError;
                return 1;
            }
            this->content += character;
            continue;
        }
        case StateChunkedNumber_CR:
        {
            this->raw += character;
            if (character != '\r')
            {
                this->error = 400;
                this->state = StateParsingError;
                return 1;
            }
            this->state = StateChunkedChunk_LF;
            continue;
        }
        case StateChunkedNumber_LF:
        {
            this->raw += character;
            if (character != '\n')
            {
                this->error = 400;
                this->state = StateParsingError;
                return 1;
            }
            this->state = StateChunkedChunk;
            continue;
        }
        case StateChunkedChunk:
        {
            this->raw += character;
            if (this->content.size() < this->number)
            {
                this->content += character;
                continue;
            }
            if (character == '\r')
            {
                this->body += this->content;
                this->content.clear();
                this->state = StateChunkedChunk_LF;
                continue;
            }
            continue;
        }
        case StateChunkedChunk_LF:
        {
            this->raw += character;
            if (character != '\n')
            {
                this->error = 400;
                this->state = StateParsingError;
                return 1;
            }
            this->raw += character;
            this->state = StateBodyChunkedNumber;
            continue;
        }
        case StateChunkedEnd_LF:
        {
            this->raw += character;
            if (character != '\n')
            {
                this->error = 400;
                this->state = StateParsingError;
                return 1;
            }
            this->state = StateParsingComplete;
            continue;
        }

        case StateParsingComplete:
        {
            {
                return 1;
            }
            Logger::info("ResourceValidator: Parsing complete. Method: " + this->method + ", URL: " + this->url);
            return 0;
        }

        default:
            std::cout << "case not handled ==> " << character << std::endl;
            this->raw += character;
            this->content += character;
        }
    }
    return 1;
}

void Request::print()
{
    std::cout << "Is Chunked: " << (this->is_chunked ? "true" : "false") << std::endl;
    std::cout << "State: " << this->state << std::endl;
    std::cout << "Method: " << this->method << std::endl;
    std::cout << "URL: " << this->url << std::endl;
    std::cout << "Version: " << this->version << std::endl;
    std::cout << "Headers:" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = this->headers.begin(); it != this->headers.end(); ++it)
    {
        std::cout << "  " << it->first << ": " << it->second << std::endl;
    }
    std::cout << "Body: " << this->body << std::endl;
    std::cout << "Error: " << this->error << std::endl;

    std::cout << "--------------------------------" << std::endl;
    std::cout << "Raw: " << this->raw << std::endl;
}

void Request::reset(void)
{
    this->headers.clear();
    this->raw.clear();
    this->state = StateMethod;
    this->method.clear();
    this->url.clear();
    this->version.clear();
    this->body.clear();
    this->content.clear();
    this->has_body = false;
    this->is_chunked = false;
    this->error = 0;
    this->number = 0;
    this->body_counter = 0;
    this->encoded_counter = 0;
    this->encoded_char.clear();
    this->headers_key.clear();
}

int Request::getBodyCounter() const { return this->body_counter; }

void Request::setBodyCounter(int bodyCounter) { this->body_counter = bodyCounter; }

int Request::getState() { return this->state; };

std::string &Request::getUrl() { return this->url; }

int Request::getError() const { return this->error; };

std::string &Request::getMethod() { return this->method; }

std::string &Request::getVersion() { return this->version; }

std::string &Request::getBody() { return this->body; };

std::map<std::string, std::string> &Request::getHeaders() { return this->headers; }

void Request::setError(int error) { this->error = error; };

void Request::setState(int state) { this->state = state; };

void Request::setHasBody(bool hasBody) { this->has_body = hasBody; }

void Request::setUrl(std::string &url) { this->url = url; }

void Request::setVersion(std::string &version) { this->version = version; }

void Request::setHeaders(std::map<std::string, std::string> &headers) { this->headers = headers; }

void Request::setBody(std::string &body) { this->body = body; }

void Request::setQueryParam(std::string &query_param) { this->query_param = query_param; };

std::string &Request::getQueryParam() { return this->query_param; };

bool Request::getHasBody() const { return this->has_body; };

Request::~Request() {};

Request::Request()
    : state(StateMethod),
      has_body(false),
      is_chunked(false),
      error(0),
      body_counter(0),
      number(0),
      encoded_counter(0)
{
    // servono nel parsing della req
    this->methods[GET] = "GET";
    this->methods[POST] = "POST";
    this->methods[DELETE] = "DELETE";
};
