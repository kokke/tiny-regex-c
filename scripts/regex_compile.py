from enum import Enum

MAX_REGEXP_LEN = 1024

class RegexType(Enum):
    UNUSED = 0
    DOT = 1
    BEGIN = 2
    END = 3
    QUESTIONMARK = 4
    STAR = 5
    PLUS = 6
    CHAR = 7
    CHAR_CLASS = 8
    INV_CHAR_CLASS = 9
    DIGIT = 10
    NOT_DIGIT = 11
    ALPHA = 12
    NOT_ALPHA = 13
    WHITESPACE = 14
    NOT_WHITESPACE = 15

class RegexSegment:
    def __init__(self, r_type, data_len=0, data=None):
        self.type = r_type
        self.data_len = data_len
        self.data = data if data else []

    def to_bytes(self):
        return bytes([self.type.value, self.data_len] + self.data)

    # def to_hex(self):
    #     if self.type in (RegexType.CHAR_CLASS, RegexType.INV_CHAR_CLASS):
    #         s = f"\\x{'[':02x}"
    #         e = f"\\x{']':02x}"
    #         content = ''.join(['\\x{0:02x}'.format(d) for d in self.data])
    #         return s + f'\\x{"^":02x}' if self.type == RegexType.INV_CHAR_CLASS else '' + content + e
    #     else:
    #         return f"\\x{self.data[0]:02x}"

    def __str__(self):
        if self.type in (RegexType.CHAR_CLASS, RegexType.INV_CHAR_CLASS):
            content = ''.join(['\\x{0:02x}'.format(d) for d in self.data])
            return f"type: {self.type.name} [{'^' if self.type == RegexType.INV_CHAR_CLASS else ''}{content}]"
        elif self.type == RegexType.CHAR:
            return f"type: {self.type.name} '\\x{self.data[0]:02x}'"
        else:
            return f"type: {self.type.name}"


def to_buffer(segments):
    # Create the flat memory buffer
    buffer = bytearray()
    for segment in segments:
        buffer.extend(segment.to_bytes())
    return buffer

# def to_hex(segments):
#     pattern = ""
#     for segment in segments:
#         pattern += segment.to_hex()
#     return pattern

def print_buffer(segments):
    print(to_buffer(segments))


class MiniRegexCompiler:
    MAX_REGEXP_LEN = 70  # Max number of bytes for a regex

    def compile(self, pattern):
        segments = []
        i = 0

        while i < len(pattern):
            c = pattern[i]
            if c == '.':
                segments.append(RegexSegment(RegexType.DOT))
            elif c == '^':
                segments.append(RegexSegment(RegexType.BEGIN))
            elif c == '$':
                segments.append(RegexSegment(RegexType.END))
            elif c == '*':
                segments.append(RegexSegment(RegexType.STAR))
            elif c == '+':
                segments.append(RegexSegment(RegexType.PLUS))
            elif c == '?':
                segments.append(RegexSegment(RegexType.QUESTIONMARK))
            elif c == '|':
                raise Exception("Unsupported")
            elif c == '\\':
                i += 1
                if i < len(pattern):
                    escaped_segment = self.handle_escape(pattern[i])
                    if escaped_segment:
                        segments.append(escaped_segment)
                    else:
                        return None  # Invalid regex
                else:
                    return None  # Invalid regex
            elif c == '[':
                char_limit = MAX_REGEXP_LEN - 4 #min(0xff, MAX_REGEXP_LEN - j - 4)
                i += 1
                if i < len(pattern) and pattern[i] == '^':
                    segment = RegexSegment(RegexType.INV_CHAR_CLASS)
                    i += 1
                    if i >= len(pattern):
                        return None
                else:
                    segment = RegexSegment(RegexType.CHAR_CLASS)

                while i < len(pattern) and pattern[i] != ']':
                    if pattern[i] == '\\':
                        i += 1
                        if i < len(pattern):
                            self.add_escaped_char(segment, pattern[i])
                        else:
                            return None  # Invalid regex
                    elif segment.data_len >= char_limit:
                        return None
                    else:
                        segment.data.append(ord(pattern[i]))
                        segment.data_len += 1
                    i += 1
                if segment.data_len >= char_limit:
                    return None

                # Character class expects 'UNUSED' at the end
                segment.data.append(RegexType.UNUSED.value)
                segment.data_len += 1
                segments.append(segment)
            elif c == '\0':
                return None
            else:
                segments.append(RegexSegment(RegexType.CHAR, 1, [ord(c)]))

            i += 1

        if len(segments) * 3 > self.MAX_REGEXP_LEN:  # Rough check, as each segment can have different lengths
            return None  # Exceeded internal buffer
                
        return segments

    def handle_escape(self, char):
        if char == 'd':
            return RegexSegment(RegexType.DIGIT)
        elif char == 'D':
            return RegexSegment(RegexType.NOT_DIGIT)
        elif char == 'w':
            return RegexSegment(RegexType.ALPHA)
        elif char == 'W':
            return RegexSegment(RegexType.NOT_ALPHA)
        elif char == 's':
            return RegexSegment(RegexType.WHITESPACE)
        elif char == 'S':
            return RegexSegment(RegexType.NOT_WHITESPACE)
        elif char in {'.', '^', '$', '*', '+', '?',  '[', ']', '\\'}: # TODO: add '|'
            return RegexSegment(RegexType.CHAR, 1, [ord(char)])
        else:
            return None  # Invalid escape sequence

    def add_escaped_char(self, segment, char):
        segment.data.append(ord('\\'))  # Add the escape character
        segment.data_len += 1
        segment.data.append(ord(char))
        segment.data_len += 1

def main():
    # Usage example
    pattern = "t*31elJ)_?~*DF_ac]*.+.*.[\\.]."
    compiler = MiniRegexCompiler()
    segments = compiler.compile(pattern)
    compiled_pattern = to_buffer(segments)
    if compiled_pattern:
        print("Compiled pattern:", compiled_pattern)
        for segment in segments:
            print(segment)
    else:
        print("Invalid regex pattern")

if __name__ == "__main__":
    main()
