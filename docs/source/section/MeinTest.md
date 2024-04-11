# This is a test

I want to test some things.
This is **strong** or *cursive* or _cursive_ or

```
Some
unformatted
text
```

(subheading-target)=
## Subheading

With some text asdfjnasöfhaöshf asjföds föijsa följsad kfjsdljflkdsajfjlks jfksa jlkds jösadj föjdsaf ölksj flkajs flksdj fölsaj fölksadj fölkjdsa fölkjdsa fölkdsa fölkadsj fölksaj 

::::{figure-md}

:::{image} ../_static/logo.png
:name: abb
:width: 50%
:::

This is my caption
::::

:::{note}
:name: notiz
This is a note.
:::

:::{warning}
This is a warning
:::

:::{important}
This is important!!!!
:::

:::{tip}
This is a tip
:::

:::{math}
:label: formel
v = ta + (1-t)b, \qquad t \in [0;1]
:::

$$ 
v + x^3 
$$ (eqn:test)

Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet.

(absatz-target)=
Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. 

Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis at vero eros et accumsan et iusto odio dignissim qui blandit praesent luptatum zzril delenit augue duis dolore te feugait nulla facilisi. Lorem ipsum dolor sit amet, consectetuer adipiscing elit, sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna aliquam erat volutpat. 

Ut wisi enim ad minim veniam, quis nostrud exerci tation ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo consequat. Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis at vero eros et accumsan et iusto odio dignissim qui blandit praesent luptatum zzril delenit augue duis dolore te feugait nulla facilisi. 

Nam liber tempor cum soluta nobis eleifend option congue nihil imperdiet doming id quod mazim placerat facer possim assum. Lorem ipsum dolor sit amet, consectetuer adipiscing elit, sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna aliquam erat volutpat. Ut wisi enim ad minim veniam, quis nostrud exerci tation ullamcorper suscipit lobortis nisl ut aliquip ex ea commodo consequat. 

Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis. 

Test fuer ein paar Referenzen, z.B. auf eine [Abbildung](#abb) oder eine [Notiz](#notiz). Dafuer muss `:name: NAME` mit angegeben werden. Fuer eine Formel sollte man `:label:` verwenden und kriegt eine Referenz der Form [](#formel), [](eqn:test). Fuer Ueberschriften (und Absaetze) kann man mit `(heading-target)=` einen Namen definieren. Dabei kommt diese Markierung **vor** dem Target. Die Referenz sieht so aus: [Ueberschrift](#subheading-target), [Absatz](#absatz-target). Links zu anderen Dokumenten (z.B. [](/getting_started.md), [](/getting_started.md#calibration)) gehen natuerlich auch.

::::{grid} 2
:::{grid-item-card}  Title 1
:class-card: intro-card
:link: ../index
:link-type: doc
:link-alt: To the user guide
A link to the api documentation page.
:::
:::{grid-item-card}  Title 2
Second card
:::
::::

```{toctree}
content.md
```