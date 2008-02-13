find ./ -name sim-im -executable -delete

COMMON_IGNORE=\
" 
moc_* 
ui_*.h 
*.o 
*.so 
qdevelop* 
"

svn propset -R svn:ignore "$COMMON_IGNORE" .
