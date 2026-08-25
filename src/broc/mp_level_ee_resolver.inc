void __cdecl Broc::EEHelper::mp_level_wad_EEGetFunctions(
        unsigned int key,
        unsigned int (__cdecl **init)(const char *),
        unsigned int (__cdecl **copy)(unsigned int),
        bool (__cdecl **equals)(unsigned int, const char *),
        void (__cdecl **dtor)(unsigned int *))
{
  if ( key > 0x5E56E3BE )
  {
    if ( key > 0xAF35F29B )
    {
      if ( key > 0xD74DC807 )
      {
        if ( key > 0xF5146A9E )
        {
          if ( key > 0xFB4614FB )
          {
            if ( key > 0xFE6B76C9 )
            {
              if ( key == -17419190 )
              {
                if ( init != nullptr )
                  *init = Broc::EEHelper::Initialize<int>;
                if ( copy != nullptr )
                  *copy = Broc::EEHelper::Copy<int>;
                if ( equals != nullptr )
                  *equals = Broc::EEHelper::Equals<int>;
                if ( dtor != nullptr )
                  *dtor = Broc::EEHelper::Destruct<int>;
              }
            }
            else
            {
              switch ( key )
              {
                case 0xFE6B76C9:
                  if ( init != nullptr )
                    *init = Broc::EEHelper::Initialize<float>;
                  if ( copy != nullptr )
                    *copy = Broc::EEHelper::Copy<float>;
                  if ( equals != nullptr )
                    *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
                  if ( dtor != nullptr )
                    *dtor = Broc::EEHelper::Destruct<float>;
                  break;
                case 0xFBCC92DF:
                  if ( init != nullptr )
                    *init = nullptr;
                  if ( copy != nullptr )
                    *copy = Broc::EEHelper::Copy<bool>;
                  if ( equals != nullptr )
                    *equals = nullptr;
                  if ( dtor != nullptr )
                    *dtor = Broc::EEHelper::Destruct<bool>;
                  break;
                case 0xFDA70120:
                  if ( init != nullptr )
                    *init = Broc::EEHelper::Initialize<int>;
                  if ( copy != nullptr )
                    *copy = Broc::EEHelper::Copy<int>;
                  if ( equals != nullptr )
                    *equals = Broc::EEHelper::Equals<int>;
                  if ( dtor != nullptr )
                    *dtor = Broc::EEHelper::Destruct<int>;
                  break;
                default:
                  break;
              }
            }
          }
          else if ( key == -79293189 )
          {
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
          }
          else if ( key > 0xF8E5A82C )
          {
            if ( key == -115163560 )
            {
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<int>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<int>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<int>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<int>;
            }
            else if ( key == -89255650 )
            {
              if ( init != nullptr )
                *init = nullptr;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<Broc::entity>;
              if ( equals != nullptr )
                *equals = nullptr;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<Broc::entity>;
            }
          }
          else
          {
            switch ( key )
            {
              case 0xF8E5A82C:
                if ( init != nullptr )
                  *init = Broc::EEHelper::Initialize<Broc::string>;
                if ( copy != nullptr )
                  *copy = Broc::EEHelper::Copy<Broc::string>;
                if ( equals != nullptr )
                  *equals = Broc::EEHelper::Equals<Broc::string>;
                if ( dtor != nullptr )
                  *dtor = Broc::EEHelper::Destruct<Broc::string>;
                break;
              case 0xF790400C:
                if ( init != nullptr )
                  *init = Broc::EEHelper::Initialize<Broc::string>;
                if ( copy != nullptr )
                  *copy = Broc::EEHelper::Copy<Broc::string>;
                if ( equals != nullptr )
                  *equals = Broc::EEHelper::Equals<Broc::string>;
                if ( dtor != nullptr )
                  *dtor = Broc::EEHelper::Destruct<Broc::string>;
                break;
              case 0xF89BEE03:
                if ( init != nullptr )
                  *init = Broc::EEHelper::Initialize<int>;
                if ( copy != nullptr )
                  *copy = Broc::EEHelper::Copy<int>;
                if ( equals != nullptr )
                  *equals = Broc::EEHelper::Equals<int>;
                if ( dtor != nullptr )
                  *dtor = Broc::EEHelper::Destruct<int>;
                break;
              default:
                break;
            }
          }
        }
        else if ( key == -183211362 )
        {
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<Broc::string>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<Broc::string>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<Broc::string>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<Broc::string>;
        }
        else if ( key > 0xE42FE83D )
        {
          if ( key > 0xED5D0E1A )
          {
            if ( key == -218764620 )
            {
              if ( init != nullptr )
                *init = nullptr;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<Broc::entity>;
              if ( equals != nullptr )
                *equals = nullptr;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<Broc::entity>;
            }
            else if ( key == -198128317 )
            {
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<int>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<int>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<int>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<int>;
            }
          }
          else
          {
            switch ( key )
            {
              case 0xED5D0E1A:
                if ( init != nullptr )
                  *init = Broc::EEHelper::Initialize<int>;
                if ( copy != nullptr )
                  *copy = Broc::EEHelper::Copy<int>;
                if ( equals != nullptr )
                  *equals = Broc::EEHelper::Equals<int>;
                if ( dtor != nullptr )
                  *dtor = Broc::EEHelper::Destruct<int>;
                break;
              case 0xEA19FC37:
                if ( init != nullptr )
                  *init = Broc::EEHelper::Initialize<Broc::string>;
                if ( copy != nullptr )
                  *copy = Broc::EEHelper::Copy<Broc::string>;
                if ( equals != nullptr )
                  *equals = Broc::EEHelper::Equals<Broc::string>;
                if ( dtor != nullptr )
                  *dtor = Broc::EEHelper::Destruct<Broc::string>;
                break;
              case 0xEA795D04:
                if ( init != nullptr )
                  *init = Broc::EEHelper::Initialize<int>;
                if ( copy != nullptr )
                  *copy = Broc::EEHelper::Copy<int>;
                if ( equals != nullptr )
                  *equals = Broc::EEHelper::Equals<int>;
                if ( dtor != nullptr )
                  *dtor = Broc::EEHelper::Destruct<int>;
                break;
              default:
                break;
            }
          }
        }
        else if ( key == -466622403 )
        {
          if ( init != nullptr )
            *init = nullptr;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<Broc::vector>;
          if ( equals != nullptr )
            *equals = nullptr;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<Broc::vector>;
        }
        else if ( key > 0xDD16B1E9 )
        {
          if ( key == -501752499 )
          {
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<Broc::string>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<Broc::string>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<Broc::string>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<Broc::string>;
          }
          else if ( key == -469500302 )
          {
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
          }
        }
        else
        {
          switch ( key )
          {
            case 0xDD16B1E9:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<int>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<int>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<int>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<int>;
              break;
            case 0xD9150A3C:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<int>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<int>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<int>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<int>;
              break;
            case 0xDAE997F9:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<Broc::string>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<Broc::string>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<Broc::string>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<Broc::string>;
              break;
            default:
              break;
          }
        }
      }
      else if ( key == -682768377 )
      {
        if ( init != nullptr )
          *init = Broc::EEHelper::Initialize<float>;
        if ( copy != nullptr )
          *copy = Broc::EEHelper::Copy<float>;
        if ( equals != nullptr )
          *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
        if ( dtor != nullptr )
          *dtor = Broc::EEHelper::Destruct<float>;
      }
      else if ( key > 0xC491D9C4 )
      {
        if ( key > 0xCF71F1AA )
        {
          if ( key > 0xD678CA45 )
          {
            if ( key == -682768631 )
            {
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<float>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<float>;
              if ( equals != nullptr )
                *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<float>;
            }
          }
          else
          {
            switch ( key )
            {
              case 0xD678CA45:
                if ( init != nullptr )
                  *init = Broc::EEHelper::Initialize<int>;
                if ( copy != nullptr )
                  *copy = Broc::EEHelper::Copy<int>;
                if ( equals != nullptr )
                  *equals = Broc::EEHelper::Equals<int>;
                if ( dtor != nullptr )
                  *dtor = Broc::EEHelper::Destruct<int>;
                break;
              case 0xD35E38B6:
                if ( init != nullptr )
                  *init = Broc::EEHelper::Initialize<int>;
                if ( copy != nullptr )
                  *copy = Broc::EEHelper::Copy<int>;
                if ( equals != nullptr )
                  *equals = Broc::EEHelper::Equals<int>;
                if ( dtor != nullptr )
                  *dtor = Broc::EEHelper::Destruct<int>;
                break;
              case 0xD472B980:
                if ( init != nullptr )
                  *init = Broc::EEHelper::Initialize<int>;
                if ( copy != nullptr )
                  *copy = Broc::EEHelper::Copy<int>;
                if ( equals != nullptr )
                  *equals = Broc::EEHelper::Equals<int>;
                if ( dtor != nullptr )
                  *dtor = Broc::EEHelper::Destruct<int>;
                break;
              default:
                break;
            }
          }
        }
        else if ( key == -814616150 )
        {
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<Broc::string>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<Broc::string>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<Broc::string>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<Broc::string>;
        }
        else if ( key > 0xC65889A7 )
        {
          if ( key == -967275867 )
          {
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<float>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<float>;
            if ( equals != nullptr )
              *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<float>;
          }
          else if ( key == -873800299 )
          {
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
          }
        }
        else
        {
          switch ( key )
          {
            case 0xC65889A7:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<float>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<float>;
              if ( equals != nullptr )
                *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<float>;
              break;
            case 0xC609EC11:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<int>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<int>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<int>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<int>;
              break;
            case 0xC655B680:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<Broc::string>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<Broc::string>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<Broc::string>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<Broc::string>;
              break;
            default:
              break;
          }
        }
      }
      else if ( key == -997074492 )
      {
        if ( init != nullptr )
          *init = Broc::EEHelper::Initialize<int>;
        if ( copy != nullptr )
          *copy = Broc::EEHelper::Copy<int>;
        if ( equals != nullptr )
          *equals = Broc::EEHelper::Equals<int>;
        if ( dtor != nullptr )
          *dtor = Broc::EEHelper::Destruct<int>;
      }
      else if ( key > 0xB8F24ACB )
      {
        if ( key > 0xBF7C8E21 )
        {
          if ( key == -1044240128 )
          {
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
          }
          else if ( key == -1008324544 )
          {
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
          }
        }
        else
        {
          switch ( key )
          {
            case 0xBF7C8E21:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<int>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<int>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<int>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<int>;
              break;
            case 0xBA634DF6:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<int>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<int>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<int>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<int>;
              break;
            case 0xBEB96D49:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<int>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<int>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<int>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<int>;
              break;
            default:
              break;
          }
        }
      }
      else if ( key == -1192080693 )
      {
        if ( init != nullptr )
          *init = Broc::EEHelper::Initialize<int>;
        if ( copy != nullptr )
          *copy = Broc::EEHelper::Copy<int>;
        if ( equals != nullptr )
          *equals = Broc::EEHelper::Equals<int>;
        if ( dtor != nullptr )
          *dtor = Broc::EEHelper::Destruct<int>;
      }
      else if ( key > 0xB46171F4 )
      {
        if ( key == -1235188682 )
        {
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<int>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<int>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<int>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<int>;
        }
        else if ( key == -1207793244 )
        {
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<int>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<int>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<int>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<int>;
        }
      }
      else
      {
        switch ( key )
        {
          case 0xB46171F4:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
            break;
          case 0xB24418D3:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<float>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<float>;
            if ( equals != nullptr )
              *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<float>;
            break;
          case 0xB27D62BF:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
            break;
          default:
            break;
        }
      }
    }
    else if ( key == -1355418981 )
    {
      if ( init != nullptr )
        *init = Broc::EEHelper::Initialize<int>;
      if ( copy != nullptr )
        *copy = Broc::EEHelper::Copy<int>;
      if ( equals != nullptr )
        *equals = Broc::EEHelper::Equals<int>;
      if ( dtor != nullptr )
        *dtor = Broc::EEHelper::Destruct<int>;
    }
    else if ( key > 0x9146436F )
    {
      if ( key > 0xA2978152 )
      {
        if ( key > 0xAB157931 )
        {
          if ( key > 0xAEB6D8F1 )
          {
            if ( key == -1357222513 )
            {
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<Broc::string>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<Broc::string>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<Broc::string>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<Broc::string>;
            }
          }
          else
          {
            switch ( key )
            {
              case 0xAEB6D8F1:
                if ( init != nullptr )
                  *init = nullptr;
                if ( copy != nullptr )
                  *copy = Broc::EEHelper::Copy<Broc::vector>;
                if ( equals != nullptr )
                  *equals = nullptr;
                if ( dtor != nullptr )
                  *dtor = Broc::EEHelper::Destruct<Broc::vector>;
                break;
              case 0xABE98552:
                if ( init != nullptr )
                  *init = Broc::EEHelper::Initialize<Broc::string>;
                if ( copy != nullptr )
                  *copy = Broc::EEHelper::Copy<Broc::string>;
                if ( equals != nullptr )
                  *equals = Broc::EEHelper::Equals<Broc::string>;
                if ( dtor != nullptr )
                  *dtor = Broc::EEHelper::Destruct<Broc::string>;
                break;
              case 0xABEEF7DA:
                if ( init != nullptr )
                  *init = Broc::EEHelper::Initialize<Broc::string>;
                if ( copy != nullptr )
                  *copy = Broc::EEHelper::Copy<Broc::string>;
                if ( equals != nullptr )
                  *equals = Broc::EEHelper::Equals<Broc::string>;
                if ( dtor != nullptr )
                  *dtor = Broc::EEHelper::Destruct<Broc::string>;
                break;
              default:
                break;
            }
          }
        }
        else if ( key == -1424656079 )
        {
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<Broc::string>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<Broc::string>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<Broc::string>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<Broc::string>;
        }
        else if ( key > 0xAA000DBC )
        {
          if ( key == -1437943561 )
          {
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<Broc::string>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<Broc::string>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<Broc::string>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<Broc::string>;
          }
          else if ( key == -1427096775 )
          {
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
          }
        }
        else
        {
          switch ( key )
          {
            case 0xAA000DBC:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<Broc::string>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<Broc::string>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<Broc::string>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<Broc::string>;
              break;
            case 0xA3572E0E:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<int>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<int>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<int>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<int>;
              break;
            case 0xA565DE41:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<int>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<int>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<int>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<int>;
              break;
            default:
              break;
          }
        }
      }
      else if ( key == -1567129262 )
      {
        if ( init != nullptr )
          *init = Broc::EEHelper::Initialize<int>;
        if ( copy != nullptr )
          *copy = Broc::EEHelper::Copy<int>;
        if ( equals != nullptr )
          *equals = Broc::EEHelper::Equals<int>;
        if ( dtor != nullptr )
          *dtor = Broc::EEHelper::Destruct<int>;
      }
      else if ( key > 0x9A8500FD )
      {
        if ( key > 0x9E624F93 )
        {
          if ( key == -1621346355 )
          {
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<Broc::string>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<Broc::string>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<Broc::string>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<Broc::string>;
          }
          else if ( key == -1616373573 )
          {
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<float>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<float>;
            if ( equals != nullptr )
              *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<float>;
          }
        }
        else
        {
          switch ( key )
          {
            case 0x9E624F93:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<Broc::string>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<Broc::string>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<Broc::string>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<Broc::string>;
              break;
            case 0x9AA56007:
              if ( init != nullptr )
                *init = nullptr;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<Broc::entity>;
              if ( equals != nullptr )
                *equals = nullptr;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<Broc::entity>;
              break;
            case 0x9C79E174:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<Broc::string>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<Broc::string>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<Broc::string>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<Broc::string>;
              break;
            default:
              break;
          }
        }
      }
      else if ( key == -1702559491 )
      {
        if ( init != nullptr )
          *init = Broc::EEHelper::Initialize<int>;
        if ( copy != nullptr )
          *copy = Broc::EEHelper::Copy<int>;
        if ( equals != nullptr )
          *equals = Broc::EEHelper::Equals<int>;
        if ( dtor != nullptr )
          *dtor = Broc::EEHelper::Destruct<int>;
      }
      else if ( key > 0x95E84605 )
      {
        if ( key == -1737189029 )
        {
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<int>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<int>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<int>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<int>;
        }
        else if ( key == -1703469790 )
        {
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<Broc::string>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<Broc::string>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<Broc::string>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<Broc::string>;
        }
      }
      else
      {
        switch ( key )
        {
          case 0x95E84605:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<float>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<float>;
            if ( equals != nullptr )
              *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<float>;
            break;
          case 0x93F860D5:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<Broc::string>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<Broc::string>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<Broc::string>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<Broc::string>;
            break;
          case 0x93FB3A03:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<Broc::string>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<Broc::string>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<Broc::string>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<Broc::string>;
            break;
          default:
            break;
        }
      }
    }
    else if ( key == -1857666193 )
    {
      if ( init != nullptr )
        *init = Broc::EEHelper::Initialize<float>;
      if ( copy != nullptr )
        *copy = Broc::EEHelper::Copy<float>;
      if ( equals != nullptr )
        *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
      if ( dtor != nullptr )
        *dtor = Broc::EEHelper::Destruct<float>;
    }
    else if ( key > 0x83C610FF )
    {
      if ( key > 0x8B17F374 )
      {
        if ( key > 0x8F2182CE )
        {
          if ( key == -1887043232 )
          {
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
          }
        }
        else
        {
          switch ( key )
          {
            case 0x8F2182CE:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<float>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<float>;
              if ( equals != nullptr )
                *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<float>;
              break;
            case 0x8CC9C547:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<int>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<int>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<int>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<int>;
              break;
            case 0x8D8B682E:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<Broc::string>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<Broc::string>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<Broc::string>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<Broc::string>;
              break;
            default:
              break;
          }
        }
      }
      else if ( key == -1961364620 )
      {
        if ( init != nullptr )
          *init = Broc::EEHelper::Initialize<int>;
        if ( copy != nullptr )
          *copy = Broc::EEHelper::Copy<int>;
        if ( equals != nullptr )
          *equals = Broc::EEHelper::Equals<int>;
        if ( dtor != nullptr )
          *dtor = Broc::EEHelper::Destruct<int>;
      }
      else if ( key > 0x87BBB1BB )
      {
        if ( key == -2012813910 )
        {
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<int>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<int>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<int>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<int>;
        }
        else if ( key == -1975034689 )
        {
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<int>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<int>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<int>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<int>;
        }
      }
      else
      {
        switch ( key )
        {
          case 0x87BBB1BB:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
            break;
          case 0x853FE9D6:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
            break;
          case 0x86F21A11:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<Broc::string>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<Broc::string>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<Broc::string>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<Broc::string>;
            break;
          default:
            break;
        }
      }
    }
    else if ( key == -2084171521 )
    {
      if ( init != nullptr )
        *init = Broc::EEHelper::Initialize<float>;
      if ( copy != nullptr )
        *copy = Broc::EEHelper::Copy<float>;
      if ( equals != nullptr )
        *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
      if ( dtor != nullptr )
        *dtor = Broc::EEHelper::Destruct<float>;
    }
    else if ( key > 0x6EFF46F7 )
    {
      if ( key > 0x75BFB68B )
      {
        if ( key == 1985257780 )
        {
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<int>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<int>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<int>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<int>;
        }
        else if ( key == -2098354766 )
        {
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<int>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<int>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<int>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<int>;
        }
      }
      else
      {
        switch ( key )
        {
          case 0x75BFB68Bu:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
            break;
          case 0x717BB65Eu:
            if ( init != nullptr )
              *init = nullptr;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<HashStr>;
            if ( equals != nullptr )
              *equals = nullptr;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<HashStr>;
            break;
          case 0x72013241u:
            if ( init != nullptr )
              *init = nullptr;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<bool>;
            if ( equals != nullptr )
              *equals = nullptr;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<bool>;
            break;
          default:
            break;
        }
      }
    }
    else if ( key == 1862223607 )
    {
      if ( init != nullptr )
        *init = Broc::EEHelper::Initialize<int>;
      if ( copy != nullptr )
        *copy = Broc::EEHelper::Copy<int>;
      if ( equals != nullptr )
        *equals = Broc::EEHelper::Equals<int>;
      if ( dtor != nullptr )
        *dtor = Broc::EEHelper::Destruct<int>;
    }
    else if ( key > 0x65D5CED3 )
    {
      if ( key == 1710938758 )
      {
        if ( init != nullptr )
          *init = Broc::EEHelper::Initialize<int>;
        if ( copy != nullptr )
          *copy = Broc::EEHelper::Copy<int>;
        if ( equals != nullptr )
          *equals = Broc::EEHelper::Equals<int>;
        if ( dtor != nullptr )
          *dtor = Broc::EEHelper::Destruct<int>;
      }
      else if ( key == 1784372508 )
      {
        if ( init != nullptr )
          *init = Broc::EEHelper::Initialize<int>;
        if ( copy != nullptr )
          *copy = Broc::EEHelper::Copy<int>;
        if ( equals != nullptr )
          *equals = Broc::EEHelper::Equals<int>;
        if ( dtor != nullptr )
          *dtor = Broc::EEHelper::Destruct<int>;
      }
    }
    else
    {
      switch ( key )
      {
        case 0x65D5CED3u:
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<float>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<float>;
          if ( equals != nullptr )
            *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<float>;
          break;
        case 0x601948FDu:
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<int>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<int>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<int>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<int>;
          break;
        case 0x64A1977Au:
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<int>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<int>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<int>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<int>;
          break;
        default:
          break;
      }
    }
  }
  else if ( key == 1582752702 )
  {
    if ( init != nullptr )
      *init = Broc::EEHelper::Initialize<int>;
    if ( copy != nullptr )
      *copy = Broc::EEHelper::Copy<int>;
    if ( equals != nullptr )
      *equals = Broc::EEHelper::Equals<int>;
    if ( dtor != nullptr )
      *dtor = Broc::EEHelper::Destruct<int>;
  }
  else if ( key > 0x308248CA )
  {
    if ( key > 0x4816A2BD )
    {
      if ( key > 0x52F516AE )
      {
        if ( key > 0x572B15ED )
        {
          if ( key > 0x58434709 )
          {
            if ( key == 1490727052 )
            {
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<float>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<float>;
              if ( equals != nullptr )
                *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<float>;
            }
          }
          else
          {
            switch ( key )
            {
              case 0x58434709u:
                if ( init != nullptr )
                  *init = Broc::EEHelper::Initialize<int>;
                if ( copy != nullptr )
                  *copy = Broc::EEHelper::Copy<int>;
                if ( equals != nullptr )
                  *equals = Broc::EEHelper::Equals<int>;
                if ( dtor != nullptr )
                  *dtor = Broc::EEHelper::Destruct<int>;
                break;
              case 0x57525C92u:
                if ( init != nullptr )
                  *init = Broc::EEHelper::Initialize<int>;
                if ( copy != nullptr )
                  *copy = Broc::EEHelper::Copy<int>;
                if ( equals != nullptr )
                  *equals = Broc::EEHelper::Equals<int>;
                if ( dtor != nullptr )
                  *dtor = Broc::EEHelper::Destruct<int>;
                break;
              case 0x5806FC2Eu:
                if ( init != nullptr )
                  *init = Broc::EEHelper::Initialize<float>;
                if ( copy != nullptr )
                  *copy = Broc::EEHelper::Copy<float>;
                if ( equals != nullptr )
                  *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
                if ( dtor != nullptr )
                  *dtor = Broc::EEHelper::Destruct<float>;
                break;
              default:
                break;
            }
          }
        }
        else if ( key == 1462441453 )
        {
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<int>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<int>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<int>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<int>;
        }
        else if ( key > 0x538E9F75 )
        {
          if ( key == 1413540660 )
          {
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<Broc::string>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<Broc::string>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<Broc::string>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<Broc::string>;
          }
          else if ( key == 1424162999 )
          {
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
          }
        }
        else
        {
          switch ( key )
          {
            case 0x538E9F75u:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<int>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<int>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<int>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<int>;
              break;
            case 0x531ABA72u:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<float>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<float>;
              if ( equals != nullptr )
                *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<float>;
              break;
            case 0x53377998u:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<float>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<float>;
              if ( equals != nullptr )
                *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<float>;
              break;
            default:
              break;
          }
        }
      }
      else if ( key == 1391793838 )
      {
        if ( init != nullptr )
          *init = Broc::EEHelper::Initialize<int>;
        if ( copy != nullptr )
          *copy = Broc::EEHelper::Copy<int>;
        if ( equals != nullptr )
          *equals = Broc::EEHelper::Equals<int>;
        if ( dtor != nullptr )
          *dtor = Broc::EEHelper::Destruct<int>;
      }
      else if ( key > 0x4A9E2402 )
      {
        if ( key > 0x4D2B5732 )
        {
          if ( key == 1296880469 )
          {
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
          }
          else if ( key == 1338137631 )
          {
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<float>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<float>;
            if ( equals != nullptr )
              *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<float>;
          }
        }
        else
        {
          switch ( key )
          {
            case 0x4D2B5732u:
              if ( init != nullptr )
                *init = nullptr;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<bool>;
              if ( equals != nullptr )
                *equals = nullptr;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<bool>;
              break;
            case 0x4B43BC2Bu:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<Broc::string>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<Broc::string>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<Broc::string>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<Broc::string>;
              break;
            case 0x4C3C14A1u:
              if ( init != nullptr )
                *init = nullptr;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<bool>;
              if ( equals != nullptr )
                *equals = nullptr;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<bool>;
              break;
            default:
              break;
          }
        }
      }
      else if ( key == 1251877890 )
      {
        if ( init != nullptr )
          *init = Broc::EEHelper::Initialize<int>;
        if ( copy != nullptr )
          *copy = Broc::EEHelper::Copy<int>;
        if ( equals != nullptr )
          *equals = Broc::EEHelper::Equals<int>;
        if ( dtor != nullptr )
          *dtor = Broc::EEHelper::Destruct<int>;
      }
      else if ( key > 0x48E740AD )
      {
        if ( key == 1247733527 )
        {
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<int>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<int>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<int>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<int>;
        }
        else if ( key == 1248944443 )
        {
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<int>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<int>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<int>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<int>;
        }
      }
      else
      {
        switch ( key )
        {
          case 0x48E740ADu:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
            break;
          case 0x4816E9E5u:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
            break;
          case 0x48254DD5u:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
            break;
          default:
            break;
        }
      }
    }
    else if ( key == 1209443005 )
    {
      if ( init != nullptr )
        *init = Broc::EEHelper::Initialize<Broc::string>;
      if ( copy != nullptr )
        *copy = Broc::EEHelper::Copy<Broc::string>;
      if ( equals != nullptr )
        *equals = Broc::EEHelper::Equals<Broc::string>;
      if ( dtor != nullptr )
        *dtor = Broc::EEHelper::Destruct<Broc::string>;
    }
    else if ( key > 0x4135B46B )
    {
      if ( key > 0x4729A3EA )
      {
        if ( key > 0x47947B55 )
        {
          if ( key == 1208989916 )
          {
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<float>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<float>;
            if ( equals != nullptr )
              *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<float>;
          }
        }
        else
        {
          switch ( key )
          {
            case 0x47947B55u:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<int>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<int>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<int>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<int>;
              break;
            case 0x4743187Eu:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<int>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<int>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<int>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<int>;
              break;
            case 0x478F159Cu:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<int>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<int>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<int>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<int>;
              break;
            default:
              break;
          }
        }
      }
      else if ( key == 1193911274 )
      {
        if ( init != nullptr )
          *init = Broc::EEHelper::Initialize<float>;
        if ( copy != nullptr )
          *copy = Broc::EEHelper::Copy<float>;
        if ( equals != nullptr )
          *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
        if ( dtor != nullptr )
          *dtor = Broc::EEHelper::Destruct<float>;
      }
      else if ( key > 0x46F0EE57 )
      {
        if ( key == 1191284611 )
        {
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<float>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<float>;
          if ( equals != nullptr )
            *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<float>;
        }
        else if ( key == 1193534728 )
        {
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<int>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<int>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<int>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<int>;
        }
      }
      else
      {
        switch ( key )
        {
          case 0x46F0EE57u:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<Broc::string>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<Broc::string>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<Broc::string>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<Broc::string>;
            break;
          case 0x42EA8664u:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
            break;
          case 0x467CC17Fu:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<float>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<float>;
            if ( equals != nullptr )
              *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<float>;
            break;
          default:
            break;
        }
      }
    }
    else if ( key == 1094038635 )
    {
      if ( init != nullptr )
        *init = Broc::EEHelper::Initialize<int>;
      if ( copy != nullptr )
        *copy = Broc::EEHelper::Copy<int>;
      if ( equals != nullptr )
        *equals = Broc::EEHelper::Equals<int>;
      if ( dtor != nullptr )
        *dtor = Broc::EEHelper::Destruct<int>;
    }
    else if ( key > 0x3A66201A )
    {
      if ( key > 0x3D43F770 )
      {
        if ( key == 1031491456 )
        {
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<int>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<int>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<int>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<int>;
        }
        else if ( key == 1094038381 )
        {
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<int>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<int>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<int>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<int>;
        }
      }
      else
      {
        switch ( key )
        {
          case 0x3D43F770u:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
            break;
          case 0x3A88015Cu:
            if ( init != nullptr )
              *init = nullptr;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<bool>;
            if ( equals != nullptr )
              *equals = nullptr;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<bool>;
            break;
          case 0x3AC811DCu:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
            break;
          default:
            break;
        }
      }
    }
    else if ( key == 979771418 )
    {
      if ( init != nullptr )
        *init = Broc::EEHelper::Initialize<int>;
      if ( copy != nullptr )
        *copy = Broc::EEHelper::Copy<int>;
      if ( equals != nullptr )
        *equals = Broc::EEHelper::Equals<int>;
      if ( dtor != nullptr )
        *dtor = Broc::EEHelper::Destruct<int>;
    }
    else if ( key > 0x36CBF8DE )
    {
      if ( key == 945124834 )
      {
        if ( init != nullptr )
          *init = nullptr;
        if ( copy != nullptr )
          *copy = Broc::EEHelper::Copy<Broc::vector>;
        if ( equals != nullptr )
          *equals = nullptr;
        if ( dtor != nullptr )
          *dtor = Broc::EEHelper::Destruct<Broc::vector>;
      }
      else if ( key == 953253096 )
      {
        if ( init != nullptr )
          *init = nullptr;
        if ( copy != nullptr )
          *copy = Broc::EEHelper::Copy<Broc::vector>;
        if ( equals != nullptr )
          *equals = nullptr;
        if ( dtor != nullptr )
          *dtor = Broc::EEHelper::Destruct<Broc::vector>;
      }
    }
    else
    {
      switch ( key )
      {
        case 0x36CBF8DEu:
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<int>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<int>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<int>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<int>;
          break;
        case 0x30CC2C00u:
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<Broc::string>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<Broc::string>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<Broc::string>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<Broc::string>;
          break;
        case 0x35249286u:
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<int>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<int>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<int>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<int>;
          break;
        default:
          break;
      }
    }
  }
  else if ( key == 813844682 )
  {
    if ( init != nullptr )
      *init = Broc::EEHelper::Initialize<int>;
    if ( copy != nullptr )
      *copy = Broc::EEHelper::Copy<int>;
    if ( equals != nullptr )
      *equals = Broc::EEHelper::Equals<int>;
    if ( dtor != nullptr )
      *dtor = Broc::EEHelper::Destruct<int>;
  }
  else if ( key > 0x1164451E )
  {
    if ( key > 0x1FA6AD2F )
    {
      if ( key > 0x2D39C958 )
      {
        if ( key > 0x2FCF6330 )
        {
          if ( key == 809239328 )
          {
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<Broc::string>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<Broc::string>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<Broc::string>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<Broc::string>;
          }
        }
        else
        {
          switch ( key )
          {
            case 0x2FCF6330u:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<int>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<int>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<int>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<int>;
              break;
            case 0x2D6AF62Au:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<int>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<int>;
              if ( equals != nullptr )
                *equals = Broc::EEHelper::Equals<int>;
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<int>;
              break;
            case 0x2F9237ECu:
              if ( init != nullptr )
                *init = Broc::EEHelper::Initialize<float>;
              if ( copy != nullptr )
                *copy = Broc::EEHelper::Copy<float>;
              if ( equals != nullptr )
                *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
              if ( dtor != nullptr )
                *dtor = Broc::EEHelper::Destruct<float>;
              break;
            default:
              break;
          }
        }
      }
      else if ( key == 758761816 )
      {
        if ( init != nullptr )
          *init = Broc::EEHelper::Initialize<int>;
        if ( copy != nullptr )
          *copy = Broc::EEHelper::Copy<int>;
        if ( equals != nullptr )
          *equals = Broc::EEHelper::Equals<int>;
        if ( dtor != nullptr )
          *dtor = Broc::EEHelper::Destruct<int>;
      }
      else if ( key > 0x26EBA853 )
      {
        if ( key == 657690791 )
        {
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<int>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<int>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<int>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<int>;
        }
        else if ( key == 740920440 )
        {
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<Broc::string>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<Broc::string>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<Broc::string>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<Broc::string>;
        }
      }
      else
      {
        switch ( key )
        {
          case 0x26EBA853u:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
            break;
          case 0x22457F30u:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
            break;
          case 0x24596F27u:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
            break;
          default:
            break;
        }
      }
    }
    else if ( key == 531017007 )
    {
      if ( init != nullptr )
        *init = Broc::EEHelper::Initialize<int>;
      if ( copy != nullptr )
        *copy = Broc::EEHelper::Copy<int>;
      if ( equals != nullptr )
        *equals = Broc::EEHelper::Equals<int>;
      if ( dtor != nullptr )
        *dtor = Broc::EEHelper::Destruct<int>;
    }
    else if ( key > 0x17B81D57 )
    {
      if ( key > 0x1C4A56B2 )
      {
        if ( key == 493170981 )
        {
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<int>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<int>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<int>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<int>;
        }
        else if ( key == 519798711 )
        {
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<float>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<float>;
          if ( equals != nullptr )
            *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<float>;
        }
      }
      else
      {
        switch ( key )
        {
          case 0x1C4A56B2u:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
            break;
          case 0x1871456Au:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
            break;
          case 0x1BA1AFE0u:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
            break;
          default:
            break;
        }
      }
    }
    else if ( key == 397942103 )
    {
      if ( init != nullptr )
        *init = Broc::EEHelper::Initialize<float>;
      if ( copy != nullptr )
        *copy = Broc::EEHelper::Copy<float>;
      if ( equals != nullptr )
        *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
      if ( dtor != nullptr )
        *dtor = Broc::EEHelper::Destruct<float>;
    }
    else if ( key > 0x149F24E8 )
    {
      if ( key == 349586970 )
      {
        if ( init != nullptr )
          *init = Broc::EEHelper::Initialize<int>;
        if ( copy != nullptr )
          *copy = Broc::EEHelper::Copy<int>;
        if ( equals != nullptr )
          *equals = Broc::EEHelper::Equals<int>;
        if ( dtor != nullptr )
          *dtor = Broc::EEHelper::Destruct<int>;
      }
      else if ( key == 366592294 )
      {
        if ( init != nullptr )
          *init = Broc::EEHelper::Initialize<int>;
        if ( copy != nullptr )
          *copy = Broc::EEHelper::Copy<int>;
        if ( equals != nullptr )
          *equals = Broc::EEHelper::Equals<int>;
        if ( dtor != nullptr )
          *dtor = Broc::EEHelper::Destruct<int>;
      }
    }
    else
    {
      switch ( key )
      {
        case 0x149F24E8u:
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<int>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<int>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<int>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<int>;
          break;
        case 0x128D01A2u:
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<Broc::string>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<Broc::string>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<Broc::string>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<Broc::string>;
          break;
        case 0x135CE3F6u:
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<Broc::string>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<Broc::string>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<Broc::string>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<Broc::string>;
          break;
        default:
          break;
      }
    }
  }
  else if ( key == 291783966 )
  {
    if ( init != nullptr )
      *init = Broc::EEHelper::Initialize<Broc::string>;
    if ( copy != nullptr )
      *copy = Broc::EEHelper::Copy<Broc::string>;
    if ( equals != nullptr )
      *equals = Broc::EEHelper::Equals<Broc::string>;
    if ( dtor != nullptr )
      *dtor = Broc::EEHelper::Destruct<Broc::string>;
  }
  else if ( key > 0x4A98EA6 )
  {
    if ( key > 0xB5B16E1 )
    {
      if ( key > 0x110004A8 )
      {
        if ( key == 290599942 )
        {
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<Broc::string>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<Broc::string>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<Broc::string>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<Broc::string>;
        }
      }
      else
      {
        switch ( key )
        {
          case 0x110004A8u:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<int>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<int>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<int>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<int>;
            break;
          case 0xF1F2946u:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<Broc::string>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<Broc::string>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<Broc::string>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<Broc::string>;
            break;
          case 0x10E8AD0Du:
            if ( init != nullptr )
              *init = Broc::EEHelper::Initialize<Broc::string>;
            if ( copy != nullptr )
              *copy = Broc::EEHelper::Copy<Broc::string>;
            if ( equals != nullptr )
              *equals = Broc::EEHelper::Equals<Broc::string>;
            if ( dtor != nullptr )
              *dtor = Broc::EEHelper::Destruct<Broc::string>;
            break;
          default:
            break;
        }
      }
    }
    else if ( key == 190519009 )
    {
      if ( init != nullptr )
        *init = Broc::EEHelper::Initialize<int>;
      if ( copy != nullptr )
        *copy = Broc::EEHelper::Copy<int>;
      if ( equals != nullptr )
        *equals = Broc::EEHelper::Equals<int>;
      if ( dtor != nullptr )
        *dtor = Broc::EEHelper::Destruct<int>;
    }
    else if ( key > 0x7AA1578 )
    {
      if ( key == 140501065 )
      {
        if ( init != nullptr )
          *init = Broc::EEHelper::Initialize<int>;
        if ( copy != nullptr )
          *copy = Broc::EEHelper::Copy<int>;
        if ( equals != nullptr )
          *equals = Broc::EEHelper::Equals<int>;
        if ( dtor != nullptr )
          *dtor = Broc::EEHelper::Destruct<int>;
      }
      else if ( key == 180881920 )
      {
        if ( init != nullptr )
          *init = Broc::EEHelper::Initialize<int>;
        if ( copy != nullptr )
          *copy = Broc::EEHelper::Copy<int>;
        if ( equals != nullptr )
          *equals = Broc::EEHelper::Equals<int>;
        if ( dtor != nullptr )
          *dtor = Broc::EEHelper::Destruct<int>;
      }
    }
    else
    {
      switch ( key )
      {
        case 0x7AA1578u:
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<int>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<int>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<int>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<int>;
          break;
        case 0x4B15AABu:
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<int>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<int>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<int>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<int>;
          break;
        case 0x514248Du:
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<float>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<float>;
          if ( equals != nullptr )
            *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<float>;
          break;
        default:
          break;
      }
    }
  }
  else if ( key == 78220966 )
  {
    if ( init != nullptr )
      *init = Broc::EEHelper::Initialize<float>;
    if ( copy != nullptr )
      *copy = Broc::EEHelper::Copy<float>;
    if ( equals != nullptr )
      *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
    if ( dtor != nullptr )
      *dtor = Broc::EEHelper::Destruct<float>;
  }
  else if ( key > 0x22BC64E )
  {
    if ( key > 0x38572C2 )
    {
      if ( key == 74805188 )
      {
        if ( init != nullptr )
          *init = Broc::EEHelper::Initialize<Broc::string>;
        if ( copy != nullptr )
          *copy = Broc::EEHelper::Copy<Broc::string>;
        if ( equals != nullptr )
          *equals = Broc::EEHelper::Equals<Broc::string>;
        if ( dtor != nullptr )
          *dtor = Broc::EEHelper::Destruct<Broc::string>;
      }
      else if ( key == 78220712 )
      {
        if ( init != nullptr )
          *init = Broc::EEHelper::Initialize<float>;
        if ( copy != nullptr )
          *copy = Broc::EEHelper::Copy<float>;
        if ( equals != nullptr )
          *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
        if ( dtor != nullptr )
          *dtor = Broc::EEHelper::Destruct<float>;
      }
    }
    else
    {
      switch ( key )
      {
        case 0x38572C2u:
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<int>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<int>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<int>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<int>;
          break;
        case 0x22F9A7Bu:
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<Broc::string>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<Broc::string>;
          if ( equals != nullptr )
            *equals = Broc::EEHelper::Equals<Broc::string>;
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<Broc::string>;
          break;
        case 0x2312FA9u:
          if ( init != nullptr )
            *init = Broc::EEHelper::Initialize<float>;
          if ( copy != nullptr )
            *copy = Broc::EEHelper::Copy<float>;
          if ( equals != nullptr )
            *equals = reinterpret_cast<bool (__cdecl *)(unsigned int, const char*)>(Broc::EEHelper::Equals<float>);
          if ( dtor != nullptr )
            *dtor = Broc::EEHelper::Destruct<float>;
          break;
        default:
          break;
      }
    }
  }
  else if ( key == 36423246 )
  {
    if ( init != nullptr )
      *init = Broc::EEHelper::Initialize<int>;
    if ( copy != nullptr )
      *copy = Broc::EEHelper::Copy<int>;
    if ( equals != nullptr )
      *equals = Broc::EEHelper::Equals<int>;
    if ( dtor != nullptr )
      *dtor = Broc::EEHelper::Destruct<int>;
  }
  else if ( key > 0x153A246 )
  {
    if ( key == 36005645 )
    {
      if ( init != nullptr )
        *init = Broc::EEHelper::Initialize<Broc::string>;
      if ( copy != nullptr )
        *copy = Broc::EEHelper::Copy<Broc::string>;
      if ( equals != nullptr )
        *equals = Broc::EEHelper::Equals<Broc::string>;
      if ( dtor != nullptr )
        *dtor = Broc::EEHelper::Destruct<Broc::string>;
    }
    else if ( key == 36191999 )
    {
      if ( init != nullptr )
        *init = Broc::EEHelper::Initialize<Broc::string>;
      if ( copy != nullptr )
        *copy = Broc::EEHelper::Copy<Broc::string>;
      if ( equals != nullptr )
        *equals = Broc::EEHelper::Equals<Broc::string>;
      if ( dtor != nullptr )
        *dtor = Broc::EEHelper::Destruct<Broc::string>;
    }
  }
  else if ( key == 22258246 )
  {
    if ( init != nullptr )
      *init = Broc::EEHelper::Initialize<Broc::string>;
    if ( copy != nullptr )
      *copy = Broc::EEHelper::Copy<Broc::string>;
    if ( equals != nullptr )
      *equals = Broc::EEHelper::Equals<Broc::string>;
    if ( dtor != nullptr )
      *dtor = Broc::EEHelper::Destruct<Broc::string>;
  }
  else if ( key == 0x0039C6FAu )
  {
    if ( init != nullptr )
      *init = nullptr;
    if ( copy != nullptr )
      *copy = Broc::EEHelper::Copy<Broc::entity>;
    if ( equals != nullptr )
      *equals = nullptr;
    if ( dtor != nullptr )
      *dtor = Broc::EEHelper::Destruct<Broc::entity>;
  }
  else if ( key == 0x003A6023u )
  {
    if ( init != nullptr )
      *init = nullptr;
    if ( copy != nullptr )
      *copy = Broc::EEHelper::Copy<Broc::entity>;
    if ( equals != nullptr )
      *equals = nullptr;
    if ( dtor != nullptr )
      *dtor = Broc::EEHelper::Destruct<Broc::entity>;
  }
}
