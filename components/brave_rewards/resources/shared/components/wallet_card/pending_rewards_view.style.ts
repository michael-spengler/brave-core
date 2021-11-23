/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import styled from 'styled-components'

export const root = styled.div`
  color: var(--brave-palette-neutral900);
  font-size: 14px;
  line-height: 21px;
  text-align: left;

  > div {
    background: #E8F4FF;

    .brave-theme-dark & {
      background: #DFF0FE;
    }
  }

  a {
    color: var(--brave-palette-blurple500);
    font-weight: 600;
    text-decoration: none;
  }

  .rewards-payment-pending {
    padding: 7px 24px;
    display: flex;
    gap: 8px;

    .icon {
      color: var(--brave-palette-blue500);
      height: 14px;
      width: auto;
      vertical-align: middle;
    }
  }

  .rewards-payment-amount {
    font-weight: 600;

    .plus {
      margin-right: 2px;
    }
  }

  .rewards-payment-completed {
    background: #E7FDEA;
    padding: 10px 15px;
    display: flex;
    align-items: center;
    gap: 13px;

    .brave-theme-dark & {
      background: #CBF1D1;
    }

    .icon {
      height: 24px;
      width: auto;
      vertical-align: middle;
    }
  }

  .rewards-payment-processing {
    padding: 7px 24px;
  }
`
