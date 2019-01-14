# ScaleformTranslationPP

This plugin enables native translation nesting present within SkyUI, as well as English fallbacks

## How to use
* Create a translation file for your project just like you would as normal, i.e. `Data\interface\translations\skyui_se_english.txt` encoded in UCS-2 LE w/ BOM
* Indicate a nested translation using `{}`, i.e. `$SKI_INFO7{}	Unequip all armor before Group Use?\nDefault: {}`
* Pass translations between `{}` to nest them, i.e. `$SKI_INFO7{F3}` becomes `Unequip all armor before Group Use?\nDefault: F3`
* You can nest nested translations as well as mix in raw text:

**MyTranslationFile_english.txt**  
>$HelloWorld{}	Hello {}!  
>$QuickBrownFox{}{}	The quick brown fox says "{}". {}.  
>$DeathAndTaxes{}{}	{} and {}  
>$Death	Death  
>$Taxes	Taxes  

By passing `$QUICKBROWNFOX{$HelloWorld{world}}{$DeathAndTaxes{Death}{$Taxes}}` to `Debug.Notification()`, we see `The quick brown fox says "Hello world!". Death and Taxes.` as a notification.
