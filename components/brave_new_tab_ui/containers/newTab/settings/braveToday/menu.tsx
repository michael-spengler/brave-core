// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import * as React from 'react'
import styled, { css } from 'styled-components'
import { MoreVertRIcon } from 'brave-ui/components/icons'

interface MenuItemData {
  onClick: Function
  child: JSX.Element
  key: string
}

interface Props {
  menuItems: MenuItemData[]
  isOpen: boolean
  onClose: boolean
}

const Menu = styled('ul')`
  z-index: 1;
  list-style: none;
  list-style-type: none;
  margin: 0;
  position: absolute;
  width: max-content;
  min-width: 166px;
  top: 114%;
  right: 0;
  border-radius: 4px;
  box-shadow: 0px 0px 6px 0px rgba(0, 0, 0, 0.3);
  padding: 8px 0;
  background-color: ${p => p.theme.color.contextMenuBackground};
  color:  ${p => p.theme.color.contextMenuForeground};
`

const MenuItem = styled('li')`
  list-style-type: none;
  padding: 10px 18px;
  outline: none;
  font-size: 12px;

  &:hover,
  &:focus {
    background-color: ${p => p.theme.color.contextMenuHoverBackground};
    color: ${p => p.theme.color.contextMenuHoverForeground};
  }

  &:active {
    // TODO(petemill): Theme doesn't have a context menu interactive color,
    // make one and don't make entire element opaque.
    opacity: .8;
  }

  &:focus-visible {
    outline: solid 1px ${p => p.theme.color.brandBrave};
  }
`

export default function PopupMenu (props: Props) {
  // TODO: call onClose when
  // - click outside ('blur' event and popupmenu isn't in active element or active element isn't child of popup menu)
  // - lose focus (see above)
  // - press esc
  return (
    <>
    {props.isOpen && props.menuItems.length &&
      <Menu
        role='menu'
      >
        { props.menuItems.map(item =>
        <MenuItem
          key={item.key}
          role='menuitem'
          tabIndex={0}
          onClick={item.onClick.bind(null)}
        >
          {item.child}
        </MenuItem>
        )}
      </Menu>
    }
    </>
  )
}

interface TriggerProps {
  isOpen: boolean
  onTrigger: Function
}

const Trigger = styled('div')`
  position: relative;
`
const TriggerIcon = styled(MoreVertRIcon)`
  width: 20px;
  height: 20px;
  transform: rotate(90deg);
`

export const EllipsisTrigger: React.FC<TriggerProps> = (props) => {
  return (
    <Trigger>
      <IconButton
        isActive={props.isOpen}
        onClick={props.onTrigger.bind(null)}
        aria-haspopup='true'
        aria-expanded={props.isOpen ? 'true' : 'false'}
      >
        <TriggerIcon />
        {props.children}
      </IconButton>
    </Trigger>
  )
}

interface ButtonProps {
  isActive?: boolean
}
export const IconButton = styled('button')<ButtonProps>`
  appearance: none;
  position: relative;
  cursor: pointer;
  margin: 0;
  border: none;
  border-radius: 100%;
  background: none;
  padding: 2px;
  display: flex;
  align-items: center;
  justify-content: center;
  color: ${p => p.theme.color.text02};

  &:hover {
    color: ${p => p.theme.color.interactive02};
    background: rgba(160, 165, 235, 0.16);

  }
  &:focus-visible {
    box-shadow: 0 0 0 2px rgb(160, 165, 235);
  }
  ${p => p.isActive && css`
    color: rgba(76, 84, 210, 0.7);
  `}
  &:active {
    color: rgba(76, 84, 210, 0.7);
  }
`
