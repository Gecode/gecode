if(NOT DEFINED GECODE_DOXYGEN_HTML_DIR)
  message(FATAL_ERROR "GECODE_DOXYGEN_HTML_DIR is required")
endif()

function(gecode_doxygen_write_alias alias_file target_file default_hash)
  set(alias_path "${GECODE_DOXYGEN_HTML_DIR}/${alias_file}")
  set(target_path "${GECODE_DOXYGEN_HTML_DIR}/${target_file}")

  if(EXISTS "${alias_path}")
    return()
  endif()
  if(NOT EXISTS "${target_path}")
    message(WARNING
      "Cannot create Doxygen compatibility alias ${alias_file}; missing ${target_file}")
    return()
  endif()

  set(target_url "${target_file}${default_hash}")
  set(anchor_html "")
  if(NOT "${default_hash}" STREQUAL "")
    set(anchor_html "<a id=\"index__\" name=\"index__\"></a>
<a id=\"index__5F\" name=\"index__5F\"></a>
")
  endif()
  file(WRITE "${alias_path}" "<!DOCTYPE html>
<html>
<head>
<meta charset=\"utf-8\">
<meta http-equiv=\"refresh\" content=\"0;url=${target_url}\">
<title>Redirecting...</title>
<script type=\"text/javascript\">
  var hash = window.location.hash || \"${default_hash}\";
  if (hash === \"#index__\") {
    hash = \"#index__5F\";
  }
  window.location.replace(\"${target_file}\" + hash);
</script>
</head>
<body>
${anchor_html}
<p>Redirecting to <a href=\"${target_url}\">${target_url}</a>.</p>
</body>
</html>
")
endfunction()

# Doxygen 1.17 renamed the module index to topics.html.
gecode_doxygen_write_alias("modules.html" "topics.html" "")

# Doxygen 1.17 folds underscore buckets into the base member index pages.
gecode_doxygen_write_alias("functions__.html" "functions.html" "#index__5F")
gecode_doxygen_write_alias("functions_func__.html" "functions_func.html" "#index__5F")
gecode_doxygen_write_alias("functions_vars__.html" "functions_vars.html" "#index__5F")
