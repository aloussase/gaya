if exists("b:did_indent")
    finish
endif

let b:did_indent = 1

if (!has("cindent") || !has("eval"))
    finish
endif

setlocal autoindent
setlocal smartindent

setlocal indentexpr=GayaIndent(v:lnum)

function! GayaIndent(lnum)
    let current_linum = a:lnum
    let current_line = getline(current_linum)

    if current_linum == 1
        return indent(curent_linum)
    endif

    let previous_linum = current_linum - 1
    let previous_line = getline(previous_linum)

    if previous_line =~ '=>\s*$' || previous_line =~ 'do\s*$'
        return indent(previous_linum) + 2
    endif

    return indent(current_linum)
endfunction
