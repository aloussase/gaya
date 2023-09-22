if exists("b:did_indent")
    finish
endif

let b:did_indent = 1

if (!has("cindent") || !has("eval"))
    finish
endif

setlocal autoindent
setlocal smartindent

setlocal indentkeys=0{,0},0],!^F,o,O,=end
setlocal indentexpr=GayaIndent(v:lnum)

function! GayaIndent(lnum)
    let current_linum = a:lnum
    let current_line = getline(current_linum)

    if current_linum == 1
        return indent(curent_linum)
    endif

    let previous_linum = current_linum - 1
    let previous_line = getline(previous_linum)

    if current_line =~ '^\s*end\s*$'
      \ && (
        \ previous_line =~ 'do\s*$'
        \ || previous_line =~ 'cases\s*$'
        \ || previous_line =~ '^\s*for'
        \ || previous_line =~ '^\s*while'
        \ )
        return indent(previous_linum)
    endif

    if current_line =~ '^\s*end\s*$'
       \ || previous_line =~ '^\s*otherwise'
        return indent(previous_linum) - 2
    endif

    if previous_line =~ '=>\s*$'
       \ || previous_line =~ 'do\s*$'
       \ || previous_line =~ 'cases\s*$'
       \ || previous_line =~ '^\s*for'
       \ || previous_line =~ '^\s*while'
        return indent(previous_linum) + 2
    endif

    return indent(current_linum)
endfunction
