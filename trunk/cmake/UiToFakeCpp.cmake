# This script tries to create fake cpp file from ui file, for feeding it to xgettext
# The main idea is to get .po file where comments points to a real line numbers in real .ui file, not
# in some generated .moc.cpp. To achive this we will parse whole .ui file, warp all strings for translation in i18n("")
# and replace all other lines with empty lines. If we then parse the result with xgettext it will show exactly
# the same line number for each string as it was in source .ui file.

# This script is at beta stage, it assumes that:
# 1. All i18n strings are warped in <string></string> tags
# 2. There is no other tags or text in the same lines where <string></string> is located
# 3. One <string></string> per line (but multiline strings are allowed)
# This assumption might fail for some valid .ui files, but this script might still be used for
# most automaticly generated ui-files

# Copyright (c) 2009, Swami Dhyan Nataraj (Nikolay Shaplov), <n@shaplov.ru>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# Arguments: IN_FILE, OUT_FILE


cmake_policy(SET CMP0007 NEW) # do not ignore empty list item in LIST command

MACRO(STRING_TAG_TO_I18N_FUN result_name str_param)
  set(str1 "${str_param}")
  string(REGEX REPLACE "\\\\" "\\\\\\\\" str1 "${str1}") # replace backslash with double backslash
  string(REGEX REPLACE "\"" "\\\\\"" str1 "${str1}") # replace " with \"
  string(REGEX REPLACE "\n" "\\\\n" str1 "${str1}") # replace new lines with \n"

  string(REGEX REPLACE "&amp\\[SEMICOLON!!!!!!\\]" "&" str1 "${str1}")
  string(REGEX REPLACE "&lt\\[SEMICOLON!!!!!!\\]" "<" str1 "${str1}")
  string(REGEX REPLACE "&gt\\[SEMICOLON!!!!!!\\]" ">" str1 "${str1}")
  
  string(REGEX REPLACE "<string>" "i18n(\"" str1 "${str1}")
  string(REGEX REPLACE "</string>" "\");" str1 "${str1}")
  SET(${result_name} "${str1}")
ENDMACRO(STRING_TAG_TO_I18N_FUN)

FILE(REMOVE "${OUT_FILE}")

FILE(STRINGS "${IN_FILE}" ui_text)

SET(ui_text "permanent_first_line;${ui_text}")
# FOREACH skips empty elements we can't use it, LIST is claimed to respect empty line is CMP0007 set for NEW,
# but if you do SET(text "1;;2;;;;3;4;") LIST(REMOVE_AT text 0) it will skip empty element at the beginning
# of the list, and the first element of the result will be 2 (at least at cmake 2.6)
# So we add dummy first element to the list and will newer remove it. In this way LIST will respect all other 
# empty elements

string(REGEX REPLACE "\\\\\;" "[SEMICOLON!!!!!!]" ui_text "${ui_text}")
# LIST do not respect backslash quoted semicolons, so before sending file content to some idiotic tag
# to convert it back to the semicolon later


SET(buf "")
SET(empty_lines_buf "")

WHILE(NOT ui_text STREQUAL "permanent_first_line" )
  list(GET ui_text 1 str)
  list(REMOVE_AT ui_text 1)

  # Please make sure that ${str} is quoted in all operations (in all macros too)
  # If you forgot quotas, you will lost all semicolons in the text
  string(REGEX REPLACE "<string></string>" "" str "${str}") # Skip empty i18n strings
  IF (buf) # If we already started buffering multiline string, continue doing that
    SET(buf "${buf}\n${str}")
    LIST(APPEND empty_lines_buf 1)
#    SET(empty_lines_buf "${empty_lines_buf}1;")
    SET(str "")
  ENDIF(in_buf)
  
  IF (str MATCHES "<string>") # if new i18n string found, starting buffering it.
    SET(buf "${str}")
  ENDIF(str MATCHES "<string>")
  
  IF (buf MATCHES "</string>") # if end of i18n string is found, parse it, print it and print as many new lines as needed
    STRING_TAG_TO_I18N_FUN(buf "${buf}")
    string(REGEX REPLACE "\\[SEMICOLON!!!!!!\\]" ";" buf "${buf}") # see coment about idiotic [SEMICOLON!!!!!!] tag above
    FILE(APPEND "${OUT_FILE}" "${buf}\n")
    FOREACH(emty_line ${empty_lines_buf})
      FILE(APPEND "${OUT_FILE}" "\n")
    ENDFOREACH(emty_line)
    SET(buf "")
    SET(empty_lines_buf "")
  ELSE(buf MATCHES "</string>")
    IF(NOT buf)  # This string is outside of "string" tags, simply replace it by empty line
      FILE(APPEND "${OUT_FILE}" "\n")
    ENDIF(NOT buf)
  ENDIF(buf MATCHES "</string>")
  
ENDWHILE(NOT ui_text STREQUAL "permanent_first_line" )

