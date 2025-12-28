(use-modules (scheme base))
(use-modules (rnrs bytevectors))

(define (map-name op val)
  (case op
    ((push) (ash val 3))
    ((pop) (logior 1 (ash val 3)))
    ((add) (logior 2 (ash val 3)))
    ((rep) (logior 3 (ash val 3)))
    ((ext) (logior 4 8 (ash val 4)))
    ((ext-fix) (logior 4 (ash val 4)))
    ((cop) (logior 5 (ash val 3)))
    ((load) (logior 6 (ash val 3)))
    ((store) (logior 7 (ash val 3)))
    (else (display "Not an op"))))

(define (push-ext op val)
  `((push ,(map-name op (logand val #b11)))
    (ext-fix ,(ash (logand val #b11100) -2))))

(define prog
  `(,@(push-ext 'push 31)
    (rep 1)
    ,@(push-ext 'store 31)
    (rep 0)))

(define out (u8-list->bytevector (map (lambda (x) (apply map-name x)) prog)))

(call-with-output-file "out.byt"
  (lambda (port)
    (write-bytevector out port)))

(display out)
