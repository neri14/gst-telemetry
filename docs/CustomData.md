# Custom Data

Custom data can be loaded from xml file using `custom-data=<path>` argument.

Data provided this way will be available in layout under keys: `custom_KEY`

---

Root element shall be: `<custom>`

Expected format is: `<data key="KEY" type="TYPE">VALUE</key>`

Where TYPE can be: `string`, `numeric`, `boolean` - if no type is provided it defaults to `string`.

example:

```xml
<custom>
  <data key="event">Some Race</data>
  <data key="racetime" type="numeric">1234</data>
</custom>
```

